#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
namespace Native {
inline const char* soundStreamFilename(const void* data,std::size_t size,std::size_t record){if(!data || record>size || size-record<8)return nullptr;const auto*b=static_cast<const unsigned char*>(data);const auto*p=b+record+4;const std::uint32_t offset=(std::uint32_t(p[0])<<24)|(std::uint32_t(p[1])<<16)|(std::uint32_t(p[2])<<8)|p[3];if(offset>=size)return nullptr;const char* text=reinterpret_cast<const char*>(b+offset);return std::memchr(text,0,size-offset)?text:nullptr;}
}
