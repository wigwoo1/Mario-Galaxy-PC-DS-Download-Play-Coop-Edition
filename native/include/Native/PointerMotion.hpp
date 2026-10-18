#pragma once
#include <Native/KpadInput.hpp>
#include <cmath>
// Normalized displacement per accepted sample, for the original movement threshold.
struct NativePointerMotion {
 bool valid=false;float x=0,y=0;
 void prepare(KPADStatus& sample) const {
  sample.speed=0;sample.vec={0,0};
  if(sample.dpd_valid_fg>=2&&valid){
   sample.vec={sample.pos.x-x,sample.pos.y-y};
   sample.speed=std::sqrt(sample.vec.x*sample.vec.x+sample.vec.y*sample.vec.y);
  }
 }
 void commit(const KPADStatus& sample){
  valid=sample.dpd_valid_fg>=2;
  x=sample.pos.x;y=sample.pos.y;
 }
};
