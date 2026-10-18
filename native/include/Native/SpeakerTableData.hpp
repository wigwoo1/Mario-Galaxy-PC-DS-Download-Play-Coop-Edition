#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <utility>
namespace Native {
struct SpeakerTableEntry {
 uint16_t wave,release,padding;uint8_t flags,volume;std::string name;
};
class SpeakerTableData {
 std::vector<SpeakerTableEntry> entries_;
 static uint32_t u32(const uint8_t* p){return uint32_t(p[0])<<24|uint32_t(p[1])<<16|uint32_t(p[2])<<8|p[3];}
 static uint16_t u16(const uint8_t* p){return uint16_t(p[0])<<8|p[1];}
public:
 const std::vector<SpeakerTableEntry>& entries()const{return entries_;}
 bool decode(const void* bytes,size_t size){
  if(!bytes||size<16)return false;auto p=static_cast<const uint8_t*>(bytes);
  size_t count=u32(p),params=u32(p+4),names=u32(p+8);
  if(u32(p+12)!=0||params<16||names<16||params>size||names>size)return false;
  if(count>(size-params)/8||count>(size-names)/4)return false;
  std::vector<SpeakerTableEntry> candidate;candidate.reserve(count);
  for(size_t i=0;i<count;++i){
   auto q=p+params+i*8;size_t off=u32(p+names+i*4);
   if(off>=size)return false;
   auto end=static_cast<const uint8_t*>(std::memchr(p+off,0,size-off));if(!end)return false;
   candidate.push_back({u16(q),u16(q+4),u16(q+6),q[2],q[3],std::string(reinterpret_cast<const char*>(p+off),size_t(end-p-off))});
  }
  entries_=std::move(candidate);return true;
 }
};
}
