#include "paper_usb.h"
#include "lifecycle.h"
#include "usbfs_compat.h"
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
namespace paper_bridge {
static bool read_int(const std::string& p, int base, int* v) {
  std::ifstream f(p); std::string s; if (!(f >> s)) return false;
  char* e = nullptr; long x = std::strtol(s.c_str(), &e, base);
  if (!e || *e) return false; *v = static_cast<int>(x); return true;
}
static std::string read_text(const std::string& p) { std::ifstream f(p); std::string s; std::getline(f,s); return s; }
static void describe_interfaces(const std::string& sys, UsbDeviceInfo* x) {
  DIR* d = opendir(sys.c_str()); if (!d) return;
  while (dirent* e = readdir(d)) {
    const char* dot = std::strchr(e->d_name, '.');
    if (!dot || dot == e->d_name) continue;
    std::string ip = sys + e->d_name + "/";
    int cls=0, sub=0, proto=0, ifnum=-1;
    read_int(ip+"bInterfaceClass",16,&cls); read_int(ip+"bInterfaceSubClass",16,&sub); read_int(ip+"bInterfaceProtocol",16,&proto); read_int(ip+"bInterfaceNumber",16,&ifnum);
    if (ifnum < 0) ifnum = std::atoi(dot+1);
    if (cls == 2 && x->control_if < 0) x->control_if = ifnum;
    if (cls == 10 && x->data_if < 0) x->data_if = ifnum;
    DIR* ed = opendir(ip.c_str()); if (!ed) continue;
    while (dirent* q = readdir(ed)) {
      if (std::strncmp(q->d_name,"ep_",3)!=0) continue;
      std::string ep = ip + q->d_name + "/";
      int addr=0, type=0; read_int(ep+"bEndpointAddress",16,&addr); read_int(ep+"bmAttributes",16,&type);
      if ((addr & 0x80) && (type & 3) == 2 && x->bulk_in < 0) x->bulk_in = addr;
    }
    closedir(ed);
  }
  closedir(d);
}
static bool scan(const std::function<bool(const UsbDeviceInfo&)>& cb) {
  const char* root = "/sys/bus/usb/devices"; DIR* d = opendir(root); if (!d) return false;
  while (dirent* e = readdir(d)) {
    if (e->d_name[0] == '.') continue;
    std::string sys = std::string(root) + "/" + e->d_name + "/";
    int vid=0,pid=0,bus=0,addr=0;
    if (!read_int(sys+"idVendor",16,&vid)||!read_int(sys+"idProduct",16,&pid)||!read_int(sys+"busnum",10,&bus)||!read_int(sys+"devnum",10,&addr)) continue;
    char path[64]; std::snprintf(path,sizeof(path),"/dev/bus/usb/%03d/%03d",bus,addr);
    UsbDeviceInfo x{bus,addr,(uint16_t)vid,(uint16_t)pid,path}; describe_interfaces(sys,&x);
    if (!cb(x)) { closedir(d); return true; }
  }
  closedir(d); return true;
}
bool PaperUsb::Enumerate(std::string* d) const {
  int count=0; bool ok=scan([&](const UsbDeviceInfo& x){
    if (x.vid==0x0425) { ++count; std::fprintf(stdout,"Paper USB %04x:%04x %s control=%d data=%d bulk-in=0x%02x\n",x.vid,x.pid,x.path.c_str(),x.control_if,x.data_if,x.bulk_in); }
    return true;
  });
  if (d) *d="matched Paper devices="+std::to_string(count)+"; descriptor discovery only"; return ok;
}
bool PaperUsb::Find(uint16_t pid, UsbDeviceInfo* out, std::string* d) const {
  if (!out) return false; bool found=false; scan([&](const UsbDeviceInfo& x){if(x.vid==0x0425&&x.pid==pid){*out=x;found=true;return false;}return true;}); if(d)*d=found?"found":"not found"; return found;
}
bool PaperUsb::Capture(const UsbDeviceInfo& x, int duration_ms, const FrameCallback& cb, std::string* d) {
  if (duration_ms <= 0 || duration_ms > 30000 || x.bulk_in < 0) { if (d) *d = "invalid bounded capture request"; return false; }
  stop_requested_.store(false);
  int fd = open(x.path.c_str(), O_RDWR | O_CLOEXEC);
  if (fd < 0) { if (d) *d = strerror(errno); return false; }
  bool control_claimed = false, data_claimed = false;
  auto cleanup = [&] {
    if (data_claimed) ioctl(fd, USBDEVFS_RELEASEINTERFACE, &x.data_if);
    if (control_claimed) ioctl(fd, USBDEVFS_RELEASEINTERFACE, &x.control_if);
    close(fd);
  };
  std::string diag;
  if (x.control_if >= 0 && ioctl(fd, USBDEVFS_CLAIMINTERFACE, &x.control_if) < 0) { if (d) *d = std::string("claim control failed: ") + strerror(errno); cleanup(); return false; }
  control_claimed = x.control_if >= 0;
  diag += "claim-control=ok; ";
  if (x.data_if >= 0 && x.data_if != x.control_if && ioctl(fd, USBDEVFS_CLAIMINTERFACE, &x.data_if) < 0) { if (d) *d = std::string("claim data failed: ") + strerror(errno); cleanup(); return false; }
  data_claimed = x.data_if >= 0 && x.data_if != x.control_if;
  diag += "claim-data=ok; ";
  unsigned char coding[7] = {0x00, 0xc2, 0x01, 0x00, 0x00, 0x00, 0x08};
  paper_usbdevfs_ctrltransfer ctl{0x21, 0x20, 0, static_cast<unsigned short>(x.control_if < 0 ? 0 : x.control_if), 7, 1000, coding};
  if (ioctl(fd, USBDEVFS_CONTROL, &ctl) < 0) { if (d) *d = std::string("line coding failed: ") + strerror(errno); cleanup(); return false; }
  paper_usbdevfs_ctrltransfer dtr{0x21, 0x22, 1, static_cast<unsigned short>(x.control_if < 0 ? 0 : x.control_if), 0, 1000, nullptr};
  if (ioctl(fd, USBDEVFS_CONTROL, &dtr) < 0) { if (d) *d = std::string("DTR failed: ") + strerror(errno); cleanup(); return false; }
  std::vector<uint8_t> buf(16384);
  const auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(duration_ms);
  int reads = 0, last_errno = 0;
  bool ok = true;
  while (!stop_requested_.load() && !StopRequested() && std::chrono::steady_clock::now() < until) {
    paper_usbdevfs_bulktransfer bulk{static_cast<unsigned int>(x.bulk_in), static_cast<unsigned int>(buf.size()), 500, buf.data()};
    int n = ioctl(fd, USBDEVFS_BULK, &bulk);
    ++reads;
    if (n > 0 && cb) {
      try { cb(x, buf.data(), static_cast<size_t>(n)); }
      catch (...) { last_errno = ECANCELED; ok = false; break; }
    } else if (n < 0) {
      last_errno = errno;
      if (errno != ETIMEDOUT && errno != EAGAIN && errno != EINTR) { ok = false; break; }
    }
  }
  const bool stopped = stop_requested_.load() || StopRequested();
  cleanup();
  if (d) *d = diag + "reads=" + std::to_string(reads) + " last=" + std::to_string(last_errno) + "/" + strerror(last_errno);
  return ok && !stopped;
}
void PaperUsb::Stop() { stop_requested_.store(true); }
}
