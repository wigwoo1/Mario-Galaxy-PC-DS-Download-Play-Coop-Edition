#pragma once
#include "graphics.hpp"
#include <Native/GpuResources.hpp>
#include <Native/GxCommandState.hpp>

// Decode the base image and any transferred palette referenced by GX registers.
// Command-side mip/filter state remains pending.
inline TextureImage decodeCommandTexture(const Native::GxCommandState& state,
                                         unsigned slot,
                                         const Native::GpuResources& resources) {
    const auto texture = state.texture(slot);
    std::vector<uint8_t> palette;
    unsigned paletteFormat = 0;
    if (texture.format == 8 || texture.format == 9 || texture.format == 10) {
        palette = state.palette(slot,texture.format==8 ? 16 : texture.format==9 ? 256 : 16384);
        paletteFormat = state.paletteFormat(slot);
    }
    const auto size = gxEncodedSize(texture.width, texture.height, texture.format);
    const auto* bytes = resources.resolve(texture.address, size);
    const std::vector<uint8_t> encoded(bytes, bytes+size);
    auto decoded=decodeGxTexture(BinaryView(encoded), texture.width, texture.height,
                           texture.format, 0, BinaryView(palette),palette.size()/2,paletteFormat);
    const auto sampler=state.sampler(slot);decoded.wrapS=sampler.wrapS;decoded.wrapT=sampler.wrapT;decoded.minFilter=sampler.minFilter;decoded.magFilter=sampler.magFilter;
    unsigned bank=slot<4?0x80:0xa0,index=slot&3;
    auto mode0=state.bp(bank+index),mode1=state.bp(bank+4+index);
    decoded.commandLod=true;decoded.commandMinLod=(mode1&255)/16.f;decoded.commandMaxLod=((mode1>>8)&255)/16.f;
    unsigned bias=(mode0>>9)&255;decoded.commandLodBias=(bias>=128?int(bias)-256:int(bias))/32.f;
    assetCheck(decoded.commandMinLod<=decoded.commandMaxLod,"Invalid command LOD range");
    unsigned lastLevel=sampler.minFilter<2?0:unsigned(std::ceil(decoded.commandMaxLod));
    uint32_t w=texture.width,h=texture.height;uint64_t offset=size;
    for(unsigned level=1;level<=lastLevel&&(w>1||h>1);++level){
        w=std::max(1u,w/2);h=std::max(1u,h/2);
        auto bytesNeeded=gxEncodedSize(w,h,texture.format);
        uint64_t address=uint64_t(texture.address)+offset;assetCheck(address<=UINT32_MAX,"Command mip address overflow");
        auto* bytes=resources.resolve(uint32_t(address),bytesNeeded);std::vector<uint8_t> encodedMip(bytes,bytes+bytesNeeded);
        auto mip=decodeGxTexture(BinaryView(encodedMip),w,h,texture.format,0,BinaryView(palette),palette.size()/2,paletteFormat);
        decoded.mipmaps.push_back({w,h,std::move(mip.pixels)});offset+=bytesNeeded;
    }
    return decoded;
}


