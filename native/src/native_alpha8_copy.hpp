#pragma once
#include "graphics.hpp"
#include <Native/GxCommandState.hpp>

// GX_CTF_A8 stores alpha in 8x4 byte tiles; it is subsequently sampled as I8.
// The EFB format determines whether alpha exists. RGB-only modes return 255,
// and RGBA6 expands its stored six bits when read by the copy engine.
inline void writeAlpha8Copy(const TextureImage& source,const Native::GxCommandState::CopyOperation& operation,Native::GpuResources& resources){
 const auto command=operation.registers[0x52];
 const unsigned format=((command>>4)&7)|(((command>>3)&1)<<3);
 const unsigned pixelMode=operation.registers[0x43]&7;
 if(operation.display()||format!=7||(command&(1u<<9))||operation.verticalScale()||pixelMode>2)
  throw std::invalid_argument("Unsupported A8 texture-copy mode");
 const unsigned w=operation.width(),h=operation.height(),left=operation.left(),top=operation.top();
 if(source.pixels.size()!=std::size_t(source.width)*source.height||left>source.width||top>source.height||w>source.width-left||h>source.height-top)
  throw std::out_of_range("A8 copy source rectangle");
 const unsigned columns=(w+7)/8,rows=(h+3)/4;const auto stride=operation.strideBytes();
 if(stride<columns*32)throw std::invalid_argument("A8 copy row stride");
 const std::size_t extent=std::size_t(rows-1)*stride+columns*32;
 const auto* prior=resources.resolve(operation.destinationAddress(),extent);
 std::vector<std::uint8_t> encoded(prior,prior+extent);
 for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){
  const auto alpha=source.pixels[std::size_t(top+y)*source.width+left+x].a;
  const auto offset=std::size_t(y/4)*stride+(x/8)*32+(y%4)*8+x%8;
  encoded[offset]=pixelMode==1?std::uint8_t((alpha&252)|(alpha>>6)):255;
 }
 // Validate and stage the entire write first. A failed destination cannot leave
 // a partially encoded texture or a cleared source in the caller's EFB.
 resources.write(operation.destinationAddress(),encoded.data(),encoded.size());
}
