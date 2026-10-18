#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
namespace Native {
inline bool decodeStreamFileTable(const void* data,std::size_t size,std::vector<std::string>& out){
 if(!data || size<8)return false;
 const auto*b=static_cast<const unsigned char*>(data);
 if(std::memcmp(b,"bsft",4))return false;
 auto word=[&](std::size_t o){return (std::uint32_t(b[o])<<24)|(std::uint32_t(b[o+1])<<16)|(std::uint32_t(b[o+2])<<8)|b[o+3];};
 const auto count=word(4);if(count>(size-8)/4)return false;
 const std::size_t strings=8+std::size_t(count)*4;
 std::vector<std::string> next;next.reserve(count);
 for(std::uint32_t i=0;i<count;++i){const auto offset=word(8+std::size_t(i)*4);if(offset<strings || offset>=size)return false;
 const auto*start=reinterpret_cast<const char*>(b+offset);const auto*end=static_cast<const char*>(std::memchr(start,0,size-offset));if(!end)return false;
 next.emplace_back(start,end);
 }
 out.swap(next);return true;
}
}
