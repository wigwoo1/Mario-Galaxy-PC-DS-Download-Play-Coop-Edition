#include <JSystem/JAudio2/JASBNKParser.hpp>
#include <cstdint>
JASBNKParser::Ver1::TChunk* JASBNKParser::Ver1::findChunk(const void* stream,u32 id){
 if(!stream)return nullptr;
 const auto* bytes=static_cast<const u8*>(stream);
 const u32 size=nativeBNKU32(bytes+4);
 for(std::uint64_t offset=0x20;offset+8<=size;){
  const u32 chunkSize=nativeBNKU32(bytes+offset+4);
  if(nativeBNKU32(bytes+offset)==id)return reinterpret_cast<TChunk*>(const_cast<u8*>(bytes+offset));
  if(chunkSize>size-offset-8)return nullptr;
  if(nativeBNKU32(bytes+offset)==id)return reinterpret_cast<TChunk*>(const_cast<u8*>(bytes+offset));
  offset=(offset+8+chunkSize+3)&~std::uint64_t(3);
 }
 return nullptr;
}



