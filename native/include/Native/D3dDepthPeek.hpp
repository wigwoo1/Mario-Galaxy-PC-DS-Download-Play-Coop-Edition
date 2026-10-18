#pragma once
#include <Native/DepthPeek.hpp>
#include <Native/D3dEfb.hpp>
namespace Native {
class D3dDepthPeek:public DepthPeekSource {
 D3dEfb& efb;
public:
 explicit D3dDepthPeek(D3dEfb& value):efb(value){}
 std::uint32_t peek(unsigned x,unsigned y)override{
  if(x>=efb.getWidth()||y>=efb.getHeight())throw std::out_of_range("GX depth coordinate outside native EFB");
  auto values=efb.readDepth();float z=values[std::size_t(y)*efb.getWidth()+x];
  if(!std::isfinite(z)||z<0||z>1)throw std::runtime_error("Invalid rendered depth");
  // GX viewport depth units use 2^24; saturate the endpoint to the 24-bit maximum.
  double scaled=double(z)*16777216.0;return scaled>=16777215.0?0xffffffu:static_cast<std::uint32_t>(scaled);
 }
};
}
