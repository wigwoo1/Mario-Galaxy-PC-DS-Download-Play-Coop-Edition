#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <stdexcept>
namespace Native {
struct UiVertex { std::array<float,2> position{};std::array<uint8_t,4> color{255,255,255,255};std::array<std::array<float,2>,8> uv{}; };
// Layout caller supplies the format selected by original SetVertexFormat.
// This decodes its direct XY/F32, optional RGBA8, and ST/F32 stream only.
inline std::array<UiVertex,4> decodeUiQuad(const uint8_t* bytes,size_t size,bool colors,unsigned textures){
 if(!bytes || textures>8 || size!=3+4*(8+(colors?4:0)+textures*8))throw std::length_error("Invalid layout quad extent");
 if(bytes[0]!=0x80 || bytes[1]!=0 || bytes[2]!=4)throw std::invalid_argument("Expected original layout quad header");
 size_t cursor=3;auto scalar=[&](){uint32_t bits=0;for(unsigned j=0;j<4;++j)bits=(bits<<8)|bytes[cursor++];float value;std::memcpy(&value,&bits,4);if(!std::isfinite(value))throw std::invalid_argument("Non-finite layout vertex");return value;};
 std::array<UiVertex,4> vertices{};for(auto& vertex:vertices){for(auto& value:vertex.position)value=scalar();if(colors)for(auto& channel:vertex.color)channel=bytes[cursor++];for(unsigned i=0;i<textures;++i)for(auto& value:vertex.uv[i])value=scalar();}return vertices;
}
// Original CharWriter: XYZ/F32, RGBA8, ST/U16 with 15 fractional bits.
// Current screen-space renderer accepts zero-depth glyphs only.
inline std::array<UiVertex,4> decodeFontQuad(const uint8_t* bytes,size_t size){
 if(!bytes || size!=83)throw std::length_error("Invalid glyph quad extent");
 if(bytes[0]!=0x80 || bytes[1]!=0 || bytes[2]!=4)throw std::invalid_argument("Invalid glyph quad header");
 size_t cursor=3;auto scalar=[&](){uint32_t bits=0;for(unsigned i=0;i<4;++i)bits=(bits<<8)|bytes[cursor++];float value;std::memcpy(&value,&bits,4);if(!std::isfinite(value))throw std::invalid_argument("Non-finite glyph position");return value;};
 std::array<UiVertex,4> result{};for(auto& vertex:result){vertex.position[0]=scalar();vertex.position[1]=scalar();if(scalar()!=0)throw std::invalid_argument("Glyph depth requires projection integration");for(auto& color:vertex.color)color=bytes[cursor++];for(auto& uv:vertex.uv[0]){unsigned value=unsigned(bytes[cursor])<<8|bytes[cursor+1];cursor+=2;uv=float(value)/32768.f;}}
 return result;
}}

