#pragma once
#include <cstdint>
#include <cstddef>
#include <map>
#include <mutex>
#include <cstring>
#include <stdexcept>
// Borrowed backing and CPU spans must remain alive until unbound/destruction.
// CPU tokens are never host-pointer casts and are never reused during this object's lifetime.
class NativeAramMemory {
 struct Span {uint8_t* data;size_t size;};
 uint8_t* backing;size_t capacity;uint64_t next=0x80000000;
 std::map<uint32_t,Span> cpu;
 std::mutex mutex;
public:
 size_t size()const noexcept{return capacity;}
 NativeAramMemory(void* data,size_t size):backing(static_cast<uint8_t*>(data)),capacity(size){if(!data || !size || size>0x7fffffff)throw std::invalid_argument("Invalid native ARAM backing");}
 uint32_t bindCpu(void* data,size_t size){std::lock_guard<std::mutex> lock(mutex);if(!data || !size || size>0x7fffffff)return 0;uint64_t rounded=(uint64_t(size)+31)&~uint64_t(31);if(next+rounded>0x100000000ull)return 0;uint32_t token=uint32_t(next);cpu.emplace(token,Span{static_cast<uint8_t*>(data),size});next+=rounded;return token;}
 bool unbindCpu(uint32_t token){std::lock_guard<std::mutex> lock(mutex);return cpu.erase(token)!=0;}
 bool transfer(unsigned direction,uint32_t cpuAddress,uint32_t aramOffset,size_t size){
  std::lock_guard<std::mutex> lock(mutex);
  if(direction>1 || cpuAddress%32 || aramOffset%32 || size%32 || aramOffset>capacity || size>capacity-aramOffset)return false;
  auto found=cpu.upper_bound(cpuAddress);if(found==cpu.begin())return false;--found;
  size_t offset=uint64_t(cpuAddress)-found->first;
  if(offset>found->second.size || size>found->second.size-offset)return false;
  auto host=found->second.data+offset;auto aram=backing+aramOffset;
  if(direction==0)std::memmove(aram,host,size);else std::memmove(host,aram,size);
  return true;
 }
};

