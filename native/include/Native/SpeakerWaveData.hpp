#pragma once
#include <cstdint>
#include <vector>
#include <utility>
namespace Native {
struct SpeakerWave {uint32_t loopStart,loopEnd;std::vector<int16_t> samples;};
class SpeakerWaveData {
 std::vector<SpeakerWave> waves_;
 static uint32_t read(const uint8_t* p){return uint32_t(p[0])<<24|uint32_t(p[1])<<16|uint32_t(p[2])<<8|p[3];}
public:
 const std::vector<SpeakerWave>& waves()const{return waves_;}
 bool decode(const void* bytes,size_t size){
  if(!bytes||size<8)return false;auto p=static_cast<const uint8_t*>(bytes);
  size_t total=read(p),count=read(p+4);
  if(total!=size||count>(size-8)/4)return false;
  size_t header=8+count*4;std::vector<SpeakerWave> result;result.reserve(count);
  for(size_t i=0;i<count;++i){
   size_t off=read(p+8+i*4);if(off<header||off>size||size-off<12)return false;
   size_t length=read(p+off);auto start=read(p+off+4),end=read(p+off+8);
   if(length%2||length>size-off-12)return false;
   bool noLoop=start==UINT32_MAX&&end==UINT32_MAX;
   if(!noLoop&&(start>=end||end>length/2))return false;
   SpeakerWave wave{start,end,{}};wave.samples.resize(length/2);
   for(size_t j=0;j<length/2;++j){
    uint16_t value=uint16_t(p[off+12+j*2])<<8|p[off+13+j*2];
    wave.samples[j]=value<32768?int16_t(value):int16_t(int32_t(value)-65536);
   }
   result.push_back(std::move(wave));
  }
  waves_=std::move(result);return true;
 }
};
}
