#include "gaze_estimator.h"
#include "calibration_store.h"
#include <algorithm>
#include <cmath>
#include <cstring>
namespace paper_bridge {
bool GazeEstimator::LoadCalibration(const char* path) {
  Calibration left, right;
  if (!CalibrationStore::Load(path, &left, &right)) return false;
  left_ = left; right_ = right; return true;
}

bool GazeEstimator::Estimate(const PupilObservation& l, const PupilObservation& r, uint64_t ns, GazeSample* out) const {
  if (!out || !ready()) return false;
  std::memset(out,0,sizeof(*out)); out->magic=kSampleMagic; out->version=kSampleVersion; out->size=sizeof(*out); out->sequence=ns; out->monotonic_ns=ns; out->flags=kCalibrated|kDaemonHealthy;
  Point lp=left_.Map({l.x,l.y}), rp=right_.Map({r.x,r.y});
  auto clamp=[](float v){return std::max(0.f,std::min(1.f,v));};
  lp.x=clamp(lp.x);lp.y=clamp(lp.y);rp.x=clamp(rp.x);rp.y=clamp(rp.y);
  if(l.valid){out->flags|=kLeftValid;out->left_confidence=l.confidence;out->left_xyz[0]=lp.x;out->left_xyz[1]=lp.y;out->left_xyz[2]=1.f;}
  if(r.valid){out->flags|=kRightValid;out->right_confidence=r.confidence;out->right_xyz[0]=rp.x;out->right_xyz[1]=rp.y;out->right_xyz[2]=1.f;}
  if(l.valid&&r.valid){out->flags|=kFusedValid;out->fused_xyz[0]=(lp.x+rp.x)*.5f;out->fused_xyz[1]=(lp.y+rp.y)*.5f;out->fused_xyz[2]=1.f;out->fused_confidence=std::min(l.confidence,r.confidence);}
  return (out->flags & (kLeftValid|kRightValid)) != 0;
}
}
