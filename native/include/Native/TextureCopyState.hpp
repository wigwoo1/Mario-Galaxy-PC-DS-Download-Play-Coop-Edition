#pragma once
#include <cstdint>
namespace Native {
struct TextureCopyState {std::uint32_t cpTexSrc=0,cpTexSize=0,cpTexStride=0,cpTex=0;bool cpTexZ=false;};
const TextureCopyState& textureCopyState();
}
