#pragma once
#include <Game/System/SaveDataFileAccessor.hpp>
#include <vector>
#include <cstring>
namespace Native {
inline u32 envelopeRead(const u8* p){return u32(p[0])<<24|u32(p[1])<<16|u32(p[2])<<8|p[3];}
inline void envelopeWrite(u8* p,u32 v){p[0]=v>>24;p[1]=v>>16;p[2]=v>>8;p[3]=v;}
inline u32 envelopeChecksum(const u8* p,u32 size){u16 sum=0,inverse=0;for(u32 i=0;i+1<size;i+=2){u16 word=u16(p[i])<<8|p[i+1];sum+=word;inverse+=u16(~word);}return u32(sum)<<16|inverse;}
inline bool envelopeLayout(const u8* data,u32 capacity,bool disk,u32& count,u32& size){
 if(!data||capacity<16)return false;
 auto word=[&](u32 offset){u32 v;std::memcpy(&v,data+offset,4);return disk?envelopeRead(data+offset):v;};
 count=word(8);size=word(12);if(word(4)!=2||!count||count>=24||size>=0x10000||size>capacity||size<16+count*16)return false;
 u32 last=16+count*16;
 for(u32 i=0;i<count;++i){const u8* name=data+16+i*16;if(!std::memchr(name,0,12)||name[0]==0)return false;u32 offset=word(28+i*16);if(offset<last||offset>=size)return false;last=offset;
 for(u32 j=0;j<i;++j)if(std::strncmp(reinterpret_cast<const char*>(name),reinterpret_cast<const char*>(data+16+j*16),12)==0)return false;}
 return true;
}
inline bool encodeSaveEnvelope(const u8* memory,u32 capacity,std::vector<u8>& disk){
 u32 count,size;if(!envelopeLayout(memory,capacity,false,count,size))return false;
 std::vector<u8> result((size+31)&~31u,0);std::memcpy(result.data(),memory,size);
 auto convert=[&](u32 offset){u32 value;std::memcpy(&value,memory+offset,4);envelopeWrite(result.data()+offset,value);};
 for(u32 offset=4;offset<16;offset+=4)convert(offset);
 for(u32 i=0;i<count;++i)convert(28+i*16);
 envelopeWrite(result.data(),envelopeChecksum(result.data()+4,size-4));disk=std::move(result);return true;
}
inline bool decodeSaveEnvelope(const u8* disk,u32 bytes,std::vector<u8>& memory){
 u32 count,size;if(!envelopeLayout(disk,bytes,true,count,size)||bytes!=((size+31)&~31u)||envelopeRead(disk)!=envelopeChecksum(disk+4,size-4))return false;
 std::vector<u8> result(disk,disk+bytes);auto convert=[&](u32 offset){u32 value=envelopeRead(disk+offset);std::memcpy(result.data()+offset,&value,4);};
 for(u32 offset=0;offset<16;offset+=4)convert(offset);
 for(u32 i=0;i<count;++i)convert(28+i*16);
 memory=std::move(result);return true;
}
}
