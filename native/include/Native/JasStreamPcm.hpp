#pragma once
#include <Native/JasStreamEndian.hpp>
#include <vector>
namespace Native {
inline bool decodeJasStreamPcm16(const void* data,std::size_t size,unsigned channels,u32 maxBlock,std::size_t frames,std::vector<s16>& out){
 JASAramStream::BlockHeader header{};if(!decodeJasStreamBlock(data,size,channels,maxBlock,header) || frames>header._4/2)return false;
 if(frames>std::vector<s16>().max_size()/channels)return false;
 const auto*b=static_cast<const unsigned char*>(data)+32;std::vector<s16> next(frames*channels);
 for(unsigned ch=0;ch<channels;++ch)for(std::size_t frame=0;frame<frames;++frame){const auto*p=b+std::size_t(header._4)*ch+frame*2;const u16 bits=(u16(p[0])<<8)|p[1];s16 sample;std::memcpy(&sample,&bits,2);next[frame*channels+ch]=sample;}
 out.swap(next);return true;
}
}
