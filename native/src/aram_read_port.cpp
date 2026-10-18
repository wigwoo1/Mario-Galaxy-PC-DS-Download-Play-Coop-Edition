#include "native_jkr_aram_port.hpp"
#include "assets.hpp"
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
// ARAM addresses remain 32-bit offsets; output pointers are always native width.
u8* JKRAram::aramToMainRam(u32 address,u8* buffer,u32 length,JKRExpandSwitch expand,u32 limit,JKRHeap* heap,int group,u32* size){
 if(size)*size=0;
 if(!length || length%32 || address%32)return nullptr;
 try{
  std::vector<uint8_t> bytes(length);
  if(!nativeDownloadAram(address,bytes.data(),bytes.size()))return nullptr;
  bool compressed=false;
  if(expand==EXPAND_SWITCH_UNKNOWN1){
   if(bytes.size()>=4 && !std::memcmp(bytes.data(),"Yaz0",4)){bytes=decodeYaz0(bytes);compressed=true;}
   else if(bytes.size()>=4 && !std::memcmp(bytes.data(),"Yay0",4)){bytes=decodeYay0(bytes);compressed=true;}
  }
  size_t count=bytes.size();if(compressed && limit && count>limit)count=limit;
  if(!buffer){buffer=static_cast<u8*>(JKRHeap::alloc(u32(count),32,heap));if(!buffer)return nullptr;}
  if(group>=0){auto owner=JKRHeap::findFromRoot(buffer);if(owner && owner->getHeapType()==0x45585048)reinterpret_cast<JKRExpHeap::CMemBlock*>(buffer-sizeof(JKRExpHeap::CMemBlock))->newGroupId(u8(group));}
  if(count)std::memcpy(buffer,bytes.data(),count);
  if(size)*size=u32(count);
  return buffer;
 }catch(const std::exception&){return nullptr;}
}
