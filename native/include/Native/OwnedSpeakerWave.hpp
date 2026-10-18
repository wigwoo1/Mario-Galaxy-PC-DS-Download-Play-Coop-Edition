#pragma once
#include <Native/SpeakerWaveData.hpp>
#include <Game/Speaker/SpkWave.hpp>
#include <cstring>
namespace Native {
// Host-endian layout matching original offsets; owner must outlive the binding.
class OwnedSpeakerWave {
 std::vector<uint32_t> storage_;
public:
 OwnedSpeakerWave()=default;OwnedSpeakerWave(const OwnedSpeakerWave&)=delete;
 OwnedSpeakerWave& operator=(const OwnedSpeakerWave&)=delete;
 bool load(const void* bytes,size_t size){
  SpeakerWaveData decoded;if(!decoded.decode(bytes,size))return false;
  size_t total=8+decoded.waves().size()*4;
  for(const auto& wave:decoded.waves()){
   size_t add=12+wave.samples.size()*2;add=(add+3)&~size_t(3);
   if(add>UINT32_MAX||total>UINT32_MAX-add)return false;total+=add;
  }
  std::vector<uint32_t> candidate(total/4);candidate[0]=uint32_t(total);candidate[1]=uint32_t(decoded.waves().size());
  size_t off=8+decoded.waves().size()*4;
  for(size_t i=0;i<decoded.waves().size();++i){
   const auto& wave=decoded.waves()[i];candidate[2+i]=uint32_t(off);
   candidate[off/4]=uint32_t(wave.samples.size()*2);
   candidate[off/4+1]=wave.loopStart;candidate[off/4+2]=wave.loopEnd;
   if(!wave.samples.empty())std::memcpy(reinterpret_cast<uint8_t*>(candidate.data())+off+12,wave.samples.data(),wave.samples.size()*2);
   off+=(12+wave.samples.size()*2+3)&~size_t(3);
  }
  storage_=std::move(candidate);return true;
 }
 size_t count()const{return storage_.size()<2?0:storage_[1];}
 void bind(SpkWave& wave){wave.mResource=storage_.empty()?nullptr:storage_.data();}
 WaveData* view(size_t index){
  if(storage_.size()<2||index>=storage_[1])return nullptr;
  return reinterpret_cast<WaveData*>(reinterpret_cast<uint8_t*>(storage_.data())+storage_[2+index]);
 }
};
}

