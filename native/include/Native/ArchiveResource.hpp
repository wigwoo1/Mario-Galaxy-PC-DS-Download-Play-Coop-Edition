#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <JSystem/JKernel/JKRArchive.hpp>
namespace Native {
inline std::vector<std::uint8_t> copyArchiveResource(JKRArchive& archive,const void* resource){
 if(!resource||archive.getResSize(resource)<0)throw std::invalid_argument("Resource does not belong to archive");
 const auto size=archive.getExpandedResSize(resource);
 if(size==0xffffffffu)throw std::invalid_argument("Archive resource extent unavailable");
 const auto* bytes=static_cast<const std::uint8_t*>(resource);
 return {bytes,bytes+size};
}
}
