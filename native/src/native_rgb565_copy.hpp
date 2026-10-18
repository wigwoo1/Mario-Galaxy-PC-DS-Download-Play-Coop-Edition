#pragma once
#include "graphics.hpp"
#include <Native/GxCommandState.hpp>
// Native RGB565 texture destination encoding. Filtering/resampling and source
// clear must be handled by the EFB pipeline before this write step.
inline void writeRgb565Copy(const TextureImage& source,const Native::GxCommandState::CopyOperation& operation,Native::GpuResources& resources){
 const auto command=operation.registers[0x52];
 const unsigned format=((command>>4)&7)|(((command>>3)&1)<<3);
 if(operation.display()||format!=4||(command&(1u<<9))||operation.verticalScale()){
  char detail[320];std::snprintf(detail,sizeof(detail),"RGB565 copy destination mode: command=%06X format=%u source=%u,%u %ux%u stride=%u destination=%08X pixelMode=%06X yscale=%06X",command,format,operation.left(),operation.top(),operation.width(),operation.height(),operation.strideBytes(),operation.destinationAddress(),operation.registers[0x43],operation.registers[0x4e]);
  throw std::invalid_argument(detail);
 }
 const unsigned w=operation.width(),h=operation.height(),left=operation.left(),top=operation.top();
 if(source.pixels.size()!=std::size_t(source.width)*source.height||left>source.width||top>source.height||w>source.width-left||h>source.height-top)throw std::out_of_range("RGB565 copy source rectangle");
 const unsigned columns=(w+3)/4,rows=(h+3)/4;const auto stride=operation.strideBytes();
 if(stride<columns*32)throw std::invalid_argument("RGB565 copy row stride");
 const std::size_t extent=std::size_t(rows-1)*stride+columns*32;
 // Preserve padding between tile rows and validate the entire destination first.
 const auto* prior=resources.resolve(operation.destinationAddress(),extent);
 std::vector<std::uint8_t> encoded(prior,prior+extent);
 for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++){
  const auto p=source.pixels[std::size_t(top+y)*source.width+left+x];
  const u16 packed=(u16(p.r>>3)<<11)|(u16(p.g>>2)<<5)|(p.b>>3);
  const auto offset=std::size_t(y/4)*stride+(x/4)*32+((y%4)*4+x%4)*2;
  encoded[offset]=std::uint8_t(packed>>8);encoded[offset+1]=std::uint8_t(packed);
 }
 resources.write(operation.destinationAddress(),encoded.data(),encoded.size());
}
