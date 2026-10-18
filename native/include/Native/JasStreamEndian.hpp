#pragma once
#include <JSystem/JAudio2/JASAramStream.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
namespace Native {
inline bool jasStreamTransferAddress(u32 base,u32 slot,u32 block,u32 slots,unsigned channels,unsigned channel,u32 bytes,u32& out){if(!block || !slots || slot>=slots || channels<1 || channels>6 || channel>=channels || !bytes || bytes>block)return false;const std::uint64_t perChannel=std::uint64_t(block)*slots;if(perChannel>0x100000000ull/channels)return false;const std::uint64_t extent=perChannel*channels;if(extent>0x100000000ull || std::uint64_t(base)+extent>0x100000000ull)return false;const std::uint64_t address=std::uint64_t(base)+std::uint64_t(block)*(std::uint64_t(slots)*channel+slot);if(address+bytes>0x100000000ull)return false;out=u32(address);return true;}
inline bool jasStreamLoopLookAhead(std::uint64_t block,u32 start,u32 end,u32& out){if(start>end)return false;if(block<=end){out=u32(block);return true;}if(start==end){out=start;return true;}const std::uint64_t period=end-start;out=start+1+u32((block-end-1)%period);return true;}
inline bool jasStreamReadBufferSize(u32 block,unsigned channels,u32& out){if(!block || channels<1 || channels>6 || block>0xffffffffu-32 || block+32>0xffffffffu/channels)return false;out=(block+32)*channels;return true;}
inline bool validJasStreamBlockSize(u32 size,unsigned format){return format==0 ? size>0 && size<=0x0fffffff : size>=2;}
inline bool decodeJasStreamHeader(const void* data,std::size_t size,JASAramStream::Header& out){
 if(!data || size<64)return false;
 const auto* b=static_cast<const unsigned char*>(data);
 auto word=[&](unsigned o){return (u32(b[o])<<24)|(u32(b[o+1])<<16)|(u32(b[o+2])<<8)|b[o+3];};
 auto half=[&](unsigned o){return u16((u16(b[o])<<8)|b[o+1]);};
 JASAramStream::Header next{};std::memcpy(&next,b,64);
 next.tag=word(0);if(next.tag!=0x5354524d || half(8)>1 || half(10)!=16)return false;next.channels=half(12);next.loop=half(14);
 const u32 rate=word(16),start=word(24),end=word(28);
 if(!next.channels || next.channels>6 || !rate || rate>0x7fffffff || start>0x7fffffff || !end || end>0x7fffffff || start>end || (next.loop && start==end))return false;
 next._10=int(rate);next.loop_start=int(start);next.loop_end=int(end);next.block_size=word(32);
 out=next;return true;
}
inline bool decodeJasStreamBlock(const void* data,std::size_t size,unsigned channels,u32 maxBlock,JASAramStream::BlockHeader& out){
 if(!data || size<32 || channels<1 || channels>6)return false;
 const auto*b=static_cast<const unsigned char*>(data);
 auto word=[&](unsigned o){return (u32(b[o])<<24)|(u32(b[o+1])<<16)|(u32(b[o+2])<<8)|b[o+3];};
 JASAramStream::BlockHeader next{};next.tag=word(0);if(next.tag!=0x424c434b)return false;next._4=word(4);
 if(!next._4 || next._4>maxBlock || next._4>(size-32)/channels)return false;
 for(unsigned i=0;i<6;++i){u16 a=(u16(b[8+i*4])<<8)|b[9+i*4],c=(u16(b[10+i*4])<<8)|b[11+i*4];std::memcpy(&next._8[i]._0,&a,2);std::memcpy(&next._8[i]._2,&c,2);}
 out=next;return true;
}}








