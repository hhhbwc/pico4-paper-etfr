#include "shared_sample.h"
#include "paper_usb.h"
#include "sample_publisher.h"
#include "jpg0_parser.h"
#include "raw_jpeg_parser.h"
#include "jpeg_decoder.h"
#include "pupil_detector.h"
#include "gaze_estimator.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <ctime>
#include <cstdio>
#include <mutex>
#include <thread>
using namespace paper_bridge;
static uint64_t now_ns() { timespec t{}; clock_gettime(CLOCK_MONOTONIC, &t); return uint64_t(t.tv_sec)*1000000000ull + uint64_t(t.tv_nsec); }
int main(int argc, char** argv) {
  if (argc > 1 && std::string(argv[1]) == "--enumerate") {
    PaperUsb usb; std::string diagnostic; bool ok = usb.Enumerate(&diagnostic);
    std::cout << diagnostic << "\n"; return ok ? 0 : 1;
  }
  if (argc > 2 && std::string(argv[1]) == "--dual-record") {
    const int seconds = std::stoi(argv[2]); const char* path = argc > 3 ? argv[3] : "/data/local/tmp/paper-pico-calibration.csv"; std::mutex output; FILE* csv=std::fopen(path,"wb"); if(!csv){std::perror(path);return 5;} std::fprintf(csv,"timestamp_ns,eye,frame,x,y,radius,confidence,valid\n");
    auto worker = [&](uint16_t pid, const char* label) { PaperUsb usb; UsbDeviceInfo info; std::string msg; if(!usb.Find(pid,&info,&msg)){std::lock_guard<std::mutex> l(output);std::cerr<<label<<" "<<msg<<"\n";return;} RawJpegParser raw; size_t frame=0,bytes=0; bool ok=usb.Capture(info,seconds*1000,[&](const UsbDeviceInfo&,const uint8_t* data,size_t n){bytes+=n;std::vector<std::vector<uint8_t>> fs;raw.Feed(data,n,&fs);for(const auto& f:fs){GrayImage im;if(!DecodeJpegToGray(f.data(),f.size(),&im))continue;PupilObservation p=PupilDetector().Detect(im.pixels.data(),im.width,im.height);std::lock_guard<std::mutex> l(output);std::fprintf(csv,"%llu,%s,%zu,%.4f,%.4f,%.4f,%.5f,%d\n",(unsigned long long)now_ns(),label,++frame,p.x,p.y,p.radius,p.confidence,p.valid?1:0);std::fflush(csv);}},&msg);std::lock_guard<std::mutex> l(output);std::cerr<<label<<" "<<msg<<" frames="<<frame<<" bytes="<<bytes<<"\n";};
    std::thread left(worker,2,"left"), right(worker,3,"right"); left.join(); right.join(); std::fclose(csv); return 0;
  }
  if (argc > 1 && std::string(argv[1]) == "--dual-live") {
    if (argc < 3) { std::cerr << "usage: --dual-live <seconds> <calibration-file>\n"; return 2; }
    const int seconds = std::stoi(argv[2]);
    const char* calibration_path = argc > 3 ? argv[3] : nullptr;
    if (!calibration_path || seconds <= 0 || seconds > 300) { std::cerr << "invalid live arguments\n"; return 2; }
    SamplePublisher publisher;
    if (!publisher.Open("/data/local/tmp/paper-pico-bridge.sample")) { std::cerr << "sample open failed\n"; return 3; }
    GazeEstimator estimator;
    if (!estimator.LoadCalibration(calibration_path)) {
      GazeSample heartbeat{}; heartbeat.magic=kSampleMagic; heartbeat.version=kSampleVersion; heartbeat.size=sizeof(heartbeat); heartbeat.sequence=now_ns(); heartbeat.monotonic_ns=heartbeat.sequence; heartbeat.flags=kDaemonHealthy; PublishSample(publisher.shared(), heartbeat);
      std::cerr << "calibration unavailable; live mode refused\n"; return 4;
    }
    struct EyeState { PupilObservation observation{}; uint64_t timestamp=0; bool seen=false; };
    EyeState eyes[2]; std::mutex state_mutex;
    auto publish_heartbeat = [&] { GazeSample heartbeat{}; heartbeat.magic=kSampleMagic; heartbeat.version=kSampleVersion; heartbeat.size=sizeof(heartbeat); heartbeat.sequence=now_ns(); heartbeat.monotonic_ns=heartbeat.sequence; heartbeat.flags=kDaemonHealthy; PublishSample(publisher.shared(), heartbeat); };
    auto worker = [&](uint16_t pid, int index, const char* label) {
      PaperUsb usb; UsbDeviceInfo info; std::string msg;
      if (!usb.Find(pid, &info, &msg)) { std::cerr << label << " " << msg << "\n"; return; }
      RawJpegParser raw; size_t frames=0, valid=0; const uint64_t pair_window=50000000ull;
      usb.Capture(info, seconds*1000, [&](const UsbDeviceInfo&, const uint8_t* data, size_t n) {
        std::vector<std::vector<uint8_t>> fs; raw.Feed(data,n,&fs);
        for (const auto& f : fs) { GrayImage image; if (!DecodeJpegToGray(f.data(),f.size(),&image)) continue; PupilObservation p=PupilDetector().Detect(image.pixels.data(),image.width,image.height); ++frames; if(p.valid) ++valid; uint64_t stamp=now_ns(); std::lock_guard<std::mutex> lock(state_mutex); eyes[index]={p,stamp,true}; EyeState other=eyes[index^1]; uint64_t delta = stamp >= other.timestamp ? stamp - other.timestamp : other.timestamp - stamp; if (!p.valid || !other.seen || !other.observation.valid || delta > pair_window) continue; GazeSample sample{}; if (estimator.Estimate(eyes[0].observation, eyes[1].observation, stamp, &sample) && (sample.flags&kFusedValid)) PublishSample(publisher.shared(),sample); }
      }, &msg);
      std::cerr << label << " " << msg << " frames=" << frames << " valid=" << valid << "\n";
    };
    std::thread left(worker,2,0,"left"), right(worker,3,1,"right"); left.join(); right.join(); publish_heartbeat(); return 0;
  }
  if (argc > 1 && std::string(argv[1]) == "--dual") {
    const int seconds = argc > 2 ? std::stoi(argv[2]) : 3; std::mutex output; SamplePublisher publisher; const bool publish = publisher.Open("/data/local/tmp/paper-pico-bridge.sample");
    auto worker = [&](uint16_t pid, const char* label) { PaperUsb usb; UsbDeviceInfo info; std::string msg; if(!usb.Find(pid,&info,&msg)){std::lock_guard<std::mutex> l(output);std::cerr<<label<<" "<<msg<<"\n";return;} RawJpegParser raw; size_t bytes=0,frames=0,valid=0; auto start=std::chrono::steady_clock::now(); bool ok=usb.Capture(info,seconds*1000,[&](const UsbDeviceInfo&,const uint8_t* data,size_t n){bytes+=n;std::vector<std::vector<uint8_t>> fs;raw.Feed(data,n,&fs);for(const auto& f:fs){++frames;GrayImage im;if(DecodeJpegToGray(f.data(),f.size(),&im)){PupilObservation p=PupilDetector().Detect(im.pixels.data(),im.width,im.height);if(p.valid)++valid;if(frames<=3){std::lock_guard<std::mutex> l(output);std::cout<<label<<" frame="<<frames<<" pupil="<<p.valid<<" center="<<p.x<<","<<p.y<<" conf="<<p.confidence<<"\n";}}}},&msg);std::lock_guard<std::mutex> l(output);std::cerr<<label<<" "<<msg<<" frames="<<frames<<" valid="<<valid<<" bytes="<<bytes<<"\n";};
    std::thread left(worker,2,"left"), right(worker,3,"right"); left.join(); right.join(); if (publish) { GazeSample heartbeat{}; heartbeat.magic=kSampleMagic; heartbeat.version=kSampleVersion; heartbeat.size=sizeof(heartbeat); heartbeat.sequence=now_ns(); heartbeat.monotonic_ns=heartbeat.sequence; heartbeat.flags=kDaemonHealthy; PublishSample(publisher.shared(),heartbeat); } return 0;
  }
  if (argc > 3 && std::string(argv[1]) == "--capture") {
    int pid = std::stoi(argv[2]); int seconds = std::stoi(argv[3]); UsbDeviceInfo info; PaperUsb usb; std::string msg;
    if (!usb.Find((uint16_t)pid, &info, &msg)) { std::cerr << msg << "\n"; return 3; }
    Jpg0Parser parser; RawJpegParser raw; size_t frames=0, bytes=0;
    bool ok=usb.Capture(info, seconds*1000, [&](const UsbDeviceInfo&,const uint8_t* data,size_t n){bytes+=n; std::vector<JpegFrame> out; parser.Feed(data,n,&out); for(const auto& f:out) { ++frames; std::cout << "JPG0 seq=" << f.sequence << " bytes=" << f.jpeg.size() << "\n"; } std::vector<std::vector<uint8_t>> raw_out; raw.Feed(data,n,&raw_out); for(const auto& f:raw_out) { ++frames; if(frames<=5){char path[128]; std::snprintf(path,sizeof(path),"/data/local/tmp/paper-pico-frame-%zu.jpg",frames); FILE* fp=std::fopen(path,"wb"); if(fp){std::fwrite(f.data(),1,f.size(),fp);std::fclose(fp);}} GrayImage image; PupilObservation pupil; if(DecodeJpegToGray(f.data(),f.size(),&image)) pupil=PupilDetector().Detect(image.pixels.data(),image.width,image.height); std::cout << "RAWJPEG bytes=" << f.size() << " image=" << image.width << "x" << image.height << " pupil=" << pupil.valid << " center=" << pupil.x << "," << pupil.y << " conf=" << pupil.confidence << "\n"; }}, &msg);
    std::cerr << msg << " frames=" << frames << " bytes=" << bytes << "\n"; return ok?0:4;
  }
  if (argc > 1 && std::string(argv[1]) == "--self-test") {
    SamplePublisher pub; if (!pub.Open("/data/local/tmp/paper-pico-bridge.sample")) return 2;
    GazeSample g{}; g.magic=kSampleMagic; g.version=kSampleVersion; g.size=sizeof(g); g.monotonic_ns=now_ns(); g.flags=kCalibrated|kDaemonHealthy|kFusedValid; g.fused_xyz[2]=1.0f; PublishSample(pub.shared(),g); std::cout << "sample published\n"; return 0;
  }
  std::cerr << "paper_bridge daemon skeleton: USB claiming and vision disabled\n";
  for (;;) std::this_thread::sleep_for(std::chrono::seconds(60));
}
