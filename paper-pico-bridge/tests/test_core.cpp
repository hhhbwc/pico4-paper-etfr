#include "jpg0_parser.h"
#include "raw_jpeg_parser.h"
#include "shared_sample.h"
#include "calibration.h"
#include "pupil_detector.h"
#include "jpeg_decoder.h"
#include "gaze_estimator.h"
#include "eye_output_adapter.h"
#include "eye_hook_controller.h"
#include "calibration_fit.h"
#include "calibration_store.h"
#include <cassert>
#include <cstring>
#include <cmath>
#include <iostream>
#include <vector>
#include <fstream>
using namespace paper_bridge;
static int OriginalEyeCall(void*, long, int, void* output) {
  if (output) static_cast<uint8_t*>(output)[0] = 0x5a;
  return 17;
}

int main() {
  Jpg0Parser p; std::vector<uint8_t> rec = {'J','P','G','0',7,0,12,0,7,0,0,0,0xff,0xd8,1,2,3,0xff,0xd9};
  std::vector<JpegFrame> frames; p.Feed(rec.data(), 5, &frames); p.Feed(rec.data()+5, rec.size()-5, &frames);
  assert(frames.size() == 1 && frames[0].sequence == 7 && frames[0].jpeg.size() == 7);
  RawJpegParser raw; uint8_t stream[]={9,0xff,0xd8,1,2,0xff,0xd9,8}; std::vector<std::vector<uint8_t>> rs; raw.Feed(stream,sizeof(stream),&rs); assert(rs.size()==1&&rs[0].size()==6);
  SharedSample s; InitializeShared(&s); GazeSample g{}; g.magic=kSampleMagic; g.version=kSampleVersion; g.size=sizeof(g); g.monotonic_ns=100; g.flags=kCalibrated|kDaemonHealthy|kFusedValid; g.fused_xyz[0]=.2f; PublishSample(&s,g); GazeSample got{}; assert(ReadFreshSample(&s,110,100,&got)); assert(!ReadFreshSample(&s,201,100,&got)); assert(!ReadFreshSample(&s,99,100,&got)); g.flags=kDaemonHealthy|kCalibrated; PublishSample(&s,g); assert(!ReadFreshSample(&s,110,100,&got));
  Calibration c; std::array<Point,9> in{}, out{}; for (int i=0;i<9;++i) { in[i] = {float(i%3),float(i/3)}; out[i] = {2*in[i].x+1,3*in[i].y-1}; } assert(c.Fit(in,out)); Point mapped=c.Map({.5f,.5f});
  const char* bad_cal = "/data/local/tmp/paper-pico-bad-calibration.bin"; { std::ofstream bad(bad_cal, std::ios::binary); bad << "not-a-calibration"; } Calibration bad_left, bad_right; assert(!CalibrationStore::Load(bad_cal, &bad_left, &bad_right)); std::remove(bad_cal); assert(std::fabs(mapped.x-2.0f)<1e-4f && std::fabs(mapped.y-.5f)<1e-4f);
  const char* stored_cal = "/data/local/tmp/paper-pico-roundtrip.bin"; assert(CalibrationStore::Save(stored_cal, in, in, out)); { std::ifstream saved(stored_cal, std::ios::binary | std::ios::ate); assert(saved && saved.tellg() == 216); } Calibration loaded_left, loaded_right; assert(CalibrationStore::Load(stored_cal, &loaded_left, &loaded_right)); assert(std::fabs(loaded_left.Map(in[4]).x - out[4].x) < 1e-4f); std::remove(stored_cal);
  std::array<Point,9> degenerate{}; for (Point& point : degenerate) point = {1.0f, 1.0f}; const char* rejected_cal = "/data/local/tmp/paper-pico-rejected.bin"; assert(!CalibrationStore::Save(rejected_cal, degenerate, in, out)); std::remove(rejected_cal); degenerate[0].x = INFINITY; assert(!CalibrationStore::Save(rejected_cal, degenerate, in, out));
  std::vector<LabeledObservation> labeled; for(int i=0;i<9;++i) labeled.push_back({in[i],out[i],true}); Calibration fitted; float rmse=1; assert(FitCalibration(labeled,&fitted,&rmse)&&rmse<1e-4f);
  std::array<LabeledObservation,9> strict{}; for(int i=0;i<9;++i) strict[i]={in[i],out[i],true}; Calibration strict_fit; float strict_rmse=1; assert(FitNinePointCalibration(strict,&strict_fit,&strict_rmse)&&strict_rmse<1e-4f); strict[4].valid=false; assert(!FitNinePointCalibration(strict,&strict_fit,&strict_rmse)); strict[4].valid=true; strict[4].pupil.x=INFINITY; assert(!FitNinePointCalibration(strict,&strict_fit,&strict_rmse));
  GazeEstimator estimator; assert(estimator.FitLeft(in,out)); assert(estimator.FitRight(in,out)); GazeSample gaze{}; assert(estimator.Estimate({1,1,4,.8f,true},{1,1,4,.6f,true},200,&gaze)); assert((gaze.flags&kFusedValid)!=0 && gaze.fused_confidence>.59f);
  uint8_t object[32]{}; EyeOutputLayout layout{4,8,12,16,24,nullptr,0}; assert(ApplyFusedGaze(object,sizeof(object),gaze,layout)); float x=0;std::memcpy(&x,object+8,4);assert(x<=1.0f); assert(!ApplyFusedGaze(object,8,gaze,layout));
  SharedSample dispatch_shared; InitializeShared(&dispatch_shared); PublishSample(&dispatch_shared, gaze); uint8_t dispatch_out[32]{}; assert(DispatchEyeTrackingData(OriginalEyeCall,nullptr,1,2,dispatch_out,sizeof(dispatch_out),&dispatch_shared,201,1000000000ull,layout)==0); assert(dispatch_out[4]==1);
  GazeSample stale=gaze; stale.monotonic_ns=1; PublishSample(&dispatch_shared,stale); std::memset(dispatch_out,0,sizeof(dispatch_out)); assert(DispatchEyeTrackingData(OriginalEyeCall,nullptr,1,2,dispatch_out,sizeof(dispatch_out),&dispatch_shared,201,100,layout)==17); assert(dispatch_out[0]==0x5a);
  std::ifstream jf("../paper-device-frames/frame1.jpg", std::ios::binary); if (jf) { std::vector<uint8_t> jb((std::istreambuf_iterator<char>(jf)), {}); GrayImage decoded; assert(DecodeJpegToGray(jb.data(),jb.size(),&decoded)); assert(decoded.width==320 && decoded.height==240); }
  std::vector<uint8_t> eye(64*48, 220); for (int y=17;y<31;++y) for (int x=25;x<39;++x) eye[y*64+x]=20; PupilObservation obs=PupilDetector().Detect(eye.data(),64,48); assert(obs.valid && obs.x>30 && obs.x<34 && obs.y>22 && obs.y<26);
  std::cout << "core tests passed\n"; return 0;
}
