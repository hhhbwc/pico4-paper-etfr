#include "shared_sample_reader.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
namespace paper_bridge {
SharedSampleReader::~SharedSampleReader(){Close();}
bool SharedSampleReader::Open(const char* path){if(!path)return false;fd_=open(path,O_RDONLY|O_CLOEXEC);if(fd_<0)return false;void* p=mmap(nullptr,sizeof(SharedSample),PROT_READ,MAP_SHARED,fd_,0);if(p==MAP_FAILED){close(fd_);fd_=-1;return false;}shared_=static_cast<const SharedSample*>(p);return true;}
void SharedSampleReader::Close(){if(shared_)munmap(const_cast<SharedSample*>(shared_),sizeof(SharedSample));if(fd_>=0)close(fd_);shared_=nullptr;fd_=-1;}
bool SharedSampleReader::Read(uint64_t now,uint64_t age,GazeSample* out)const{return ReadFreshSample(shared_,now,age,out);}
}
