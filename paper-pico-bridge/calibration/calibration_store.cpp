#include "calibration_store.h"
#include <cstdio>
#include <string>
#include <cmath>
namespace paper_bridge {
bool CalibrationStore::Load(const char* path, Calibration* left, Calibration* right) {
  if(!path||!left||!right) return false; FILE* f=std::fopen(path,"rb"); if(!f) return false; if(std::fseek(f,0,SEEK_END)!=0){std::fclose(f);return false;} long length=std::ftell(f); if(length!=long(27*sizeof(Point)) || std::fseek(f,0,SEEK_SET)!=0){std::fclose(f);return false;} std::array<Point,9> li{},ri{},o{}; bool ok=std::fread(li.data(),sizeof(Point),9,f)==9&&std::fread(ri.data(),sizeof(Point),9,f)==9&&std::fread(o.data(),sizeof(Point),9,f)==9; std::fclose(f); if(!ok)return false; for(const auto& p:li)if(!std::isfinite(p.x)||!std::isfinite(p.y))return false; for(const auto& p:ri)if(!std::isfinite(p.x)||!std::isfinite(p.y))return false; for(const auto& p:o)if(!std::isfinite(p.x)||!std::isfinite(p.y))return false; return left->Fit(li,o)&&right->Fit(ri,o);
}
bool CalibrationStore::Save(const char* path,const std::array<Point,9>& li,const std::array<Point,9>& ri,const std::array<Point,9>& o) {
  if(!path) return false; std::string tmp=std::string(path)+".tmp"; FILE* f=std::fopen(tmp.c_str(),"wb"); if(!f) return false; bool ok=std::fwrite(li.data(),sizeof(Point),9,f)==9&&std::fwrite(ri.data(),sizeof(Point),9,f)==9&&std::fwrite(o.data(),sizeof(Point),9,f)==9; std::fflush(f); std::fclose(f); if(!ok){std::remove(tmp.c_str());return false;} return std::rename(tmp.c_str(),path)==0;
}
}
