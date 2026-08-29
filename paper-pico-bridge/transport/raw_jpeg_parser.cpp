#include "raw_jpeg_parser.h"
#include <algorithm>
namespace paper_bridge {
void RawJpegParser::Reset() { buffer_.clear(); in_frame_=false; }
void RawJpegParser::Feed(const uint8_t* data, size_t size, std::vector<std::vector<uint8_t>>* out) {
  if (!data || !size || !out) return;
  buffer_.insert(buffer_.end(), data, data + size);
  while (true) {
    if (!in_frame_) {
      const uint8_t soi[2] = {0xff, 0xd8};
      auto it=std::search(buffer_.begin(),buffer_.end(),soi,soi+2);
      if (it==buffer_.end()) { if(buffer_.size()>1) buffer_.erase(buffer_.begin(),buffer_.end()-1); return; }
      buffer_.erase(buffer_.begin(),it); in_frame_=true;
    }
    if (buffer_.size()<4) return;
    size_t end=0; for(size_t i=2;i<buffer_.size();++i) if(buffer_[i-1]==0xff&&buffer_[i]==0xd9){end=i+1;break;}
    if (!end) { if(buffer_.size()>max_frame_){Reset();} return; }
    out->emplace_back(buffer_.begin(),buffer_.begin()+end); buffer_.erase(buffer_.begin(),buffer_.begin()+end); in_frame_=false;
  }
}
}
