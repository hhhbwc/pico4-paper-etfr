#include <sys/types.h>
#include "zygisk.hpp"
#include <android/log.h>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include "runtime_signature.h"
#include "shared_sample_reader.h"
#include "runtime_probe.h"
#define TAG "PaperPicoBridge"
static bool is_target(const char* name) { return name && std::strcmp(name,"com.pico.xr.openxr_runtime")==0; }
class PaperBridgeModule final : public zygisk::ModuleBase {
 public:
  void onLoad(zygisk::Api* api, JNIEnv* env) override { api_=api; env_=env; }
  void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
    const char* name = nullptr; if (args && args->nice_name) name = env_->GetStringUTFChars(args->nice_name, nullptr);
    if (!is_target(name)) { if(name) env_->ReleaseStringUTFChars(args->nice_name,name); api_->setOption(zygisk::DLCLOSE_MODULE_LIBRARY); return; }
    env_->ReleaseStringUTFChars(args->nice_name,name);
    __android_log_print(ANDROID_LOG_INFO,TAG,"target runtime recognized; signature-gated hook pending");
  }
  void postAppSpecialize(const zygisk::AppSpecializeArgs*) override {
    auto probe = paper_bridge::ProbeSelfRuntime(); __android_log_print(ANDROID_LOG_INFO,TAG,"runtime probe base=%p target=%p prologue=%d",(void*)probe.base,(void*)probe.get_eye_tracking_data,probe.prologue_ok?1:0);
    paper_bridge::SharedSampleReader reader; if(!reader.Open("/data/local/tmp/paper-pico-bridge.sample")){__android_log_print(ANDROID_LOG_INFO,TAG,"sample unavailable; original runtime untouched");return;}
    timespec t{}; clock_gettime(CLOCK_MONOTONIC,&t); uint64_t now=uint64_t(t.tv_sec)*1000000000ull+t.tv_nsec; paper_bridge::GazeSample sample{};
    bool fresh=reader.Read(now,100000000ull,&sample); __android_log_print(ANDROID_LOG_INFO,TAG,"sample probe fresh=%d flags=0x%x; hook disabled",fresh?1:0,sample.flags);
  }
 private: zygisk::Api* api_ = nullptr; JNIEnv* env_ = nullptr;
};
REGISTER_ZYGISK_MODULE(PaperBridgeModule)
