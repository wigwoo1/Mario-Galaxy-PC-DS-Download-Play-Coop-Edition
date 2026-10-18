#pragma once
#include <Native/SpeakerTableData.hpp>
#include <Game/Speaker/SpkTable.hpp>
namespace Native {
// Binding pointers remain valid until successful reload or destruction; rebind after reload.
class OwnedSpeakerTable {
 SpeakerTableData data_;std::vector<SpkParameters> parameters_;std::vector<const char*> names_;
public:
 OwnedSpeakerTable()=default;
 OwnedSpeakerTable(const OwnedSpeakerTable&)=delete;OwnedSpeakerTable& operator=(const OwnedSpeakerTable&)=delete;
 bool load(const void* bytes,size_t size){
  SpeakerTableData candidate;if(!candidate.decode(bytes,size))return false;
  std::vector<SpkParameters> parameters;std::vector<const char*> names;
  parameters.reserve(candidate.entries().size());names.reserve(candidate.entries().size());
  for(const auto& entry:candidate.entries())parameters.push_back({entry.wave,entry.flags,entry.volume,entry.release,entry.padding});
  data_=std::move(candidate);parameters_=std::move(parameters);
  for(const auto& entry:data_.entries())names.push_back(entry.name.c_str());
  names_=std::move(names);return true;
 }
 void bind(SpkTable& table){
  table.mResourceCount=static_cast<s32>(parameters_.size());
  table.mParameters=parameters_.data();table.mNames=names_.data();table.mInitialized=true;
 }
};
}
