#include "runtime_probe.h"
#include "runtime_signature.h"
#include <cstdio>
#include <cstring>
namespace paper_bridge {
RuntimeProbe ProbeSelfRuntime() {
  RuntimeProbe r{}; FILE* f=std::fopen("/proc/self/maps","r"); if(!f)return r; char line[512], path[256];
  uintptr_t map_end = 0;
  while(std::fgets(line,sizeof(line),f)){unsigned long long s=0,e=0,o=0;path[0]=0;if(std::sscanf(line,"%llx-%llx %*4s %llx %*s %*s %255s",&s,&e,&o,path)>=4&&std::strstr(path,"libeyetrackingclient.pxr.so")&&o==0){r.base=(uintptr_t)s;map_end=(uintptr_t)e;break;}}
  std::fclose(f); if(!r.base || map_end <= r.base || kEyeClientOffset + sizeof(kKnownGetEyeTrackingDataPrologue) > map_end-r.base)return r; r.get_eye_tracking_data=r.base+kEyeClientOffset; r.prologue_ok=std::memcmp(reinterpret_cast<const void*>(r.get_eye_tracking_data),kKnownGetEyeTrackingDataPrologue,sizeof(kKnownGetEyeTrackingDataPrologue))==0; return r;
}
}
