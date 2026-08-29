#include "pupil_detector.h"
#include <algorithm>
#include <cmath>
#include <queue>
namespace paper_bridge {
PupilObservation PupilDetector::Detect(const uint8_t* img, int w, int h) const {
  if (!img || w < 32 || h < 32) return {};
  uint64_t sum=0; for(int i=0;i<w*h;++i) sum+=img[i];
  const int threshold=std::min(100, std::max(12, int(sum/(w*h))-25));
  std::vector<uint8_t> seen(size_t(w)*h); PupilObservation best{};
  const int margin=8;
  for(int sy=margin;sy<h-margin;++sy) for(int sx=margin;sx<w-margin;++sx) {
    int start=sy*w+sx; if(seen[start]||img[start]>threshold) continue;
    std::queue<int> q; q.push(start); seen[start]=1; int count=0,minx=sx,maxx=sx,miny=sy,maxy=sy; double mx=0,my=0;
    while(!q.empty()) { int p=q.front();q.pop();int x=p%w,y=p/w;++count;mx+=x;my+=y;minx=std::min(minx,x);maxx=std::max(maxx,x);miny=std::min(miny,y);maxy=std::max(maxy,y);
      const int n[4]={p-1,p+1,p-w,p+w}; for(int v:n) {int vx=v%w,vy=v/w;if(v>=0&&v<w*h&&vx>=0&&vx<w&&vy>=0&&vy<h&&!seen[v]&&img[v]<=threshold){seen[v]=1;q.push(v);}}
    }
    float bw=maxx-minx+1.f,bh=maxy-miny+1.f,ratio=std::min(bw,bh)/std::max(bw,bh),radius=std::sqrt(count/3.1415926f);
    if(count<20||radius<3||radius>std::min(w,h)*.25f||ratio<.45f) continue;
    float score=std::min(1.f,count/600.f)*ratio;
    if(score>best.confidence) best={float(mx/count),float(my/count),radius,score,score>=.12f};
  }
  return best;
}
}
