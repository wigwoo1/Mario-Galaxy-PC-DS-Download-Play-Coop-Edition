#pragma once
#include <Native/SpeakerTableData.hpp>
#include <Native/SpeakerWaveData.hpp>
namespace Native {
class SpeakerResources {
 SpeakerTableData table_;SpeakerWaveData wave_;
public:
 const SpeakerTableData& table()const{return table_;}
 const SpeakerWaveData& wave()const{return wave_;}
 bool decode(const void* table,size_t tableBytes,const void* wave,size_t waveBytes){
  SpeakerTableData t;SpeakerWaveData w;
  if(!t.decode(table,tableBytes)||!w.decode(wave,waveBytes))return false;
  for(const auto& entry:t.entries())if(entry.wave>=w.waves().size())return false;
  table_=std::move(t);wave_=std::move(w);return true;
 }
};
}
