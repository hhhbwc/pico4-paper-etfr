#include "calibration_fit.h"
#include <cmath>
namespace paper_bridge {
bool FitCalibration(const std::vector<LabeledObservation>& obs, Calibration* result, float* rmse) {
  if (!result || obs.size() < 3) return false;
  std::array<Point,9> in{}, out{}; size_t n=0;
  for (const auto& o:obs) if(o.valid && n<9){in[n]=o.pupil;out[n]=o.target;++n;}
  if(n<3) return false; for(size_t i=n;i<9;++i){in[i]=in[n-1];out[i]=out[n-1];}
  if(!result->Fit(in,out)) return false; double e=0; size_t count=0;
  for(const auto& o:obs) if(o.valid){Point p=result->Map(o.pupil);double dx=p.x-o.target.x,dy=p.y-o.target.y;e+=dx*dx+dy*dy;++count;}
  if(rmse)*rmse=count?static_cast<float>(std::sqrt(e/count)):0.f; return true;
}
}
