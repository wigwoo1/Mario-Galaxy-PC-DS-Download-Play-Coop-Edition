#include <Native/GpuResources.hpp>
#include "native_font_records.hpp"
#include <nw4r/ut/ResFont.h>
#include <Native/FontResource.hpp>
#include <cassert>
#include <fstream>
#include <iterator>
#include <cstdio>
int main(int argc,char** argv){
    assert(argc==2);
    std::ifstream file(argv[1],std::ios::binary);assert(file);
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});
    auto reference=decodeResourceFont(BinaryView(bytes));
    nw4r::ut::ResFont font;
    assert(!font.SetResource(reinterpret_cast<void*>(uintptr_t(1))));
    assert(!nativeRegisterFontResource(nullptr,16));
    assert(nativeRegisterFontResource(bytes.data(),bytes.size()));
    assert(!nativeRegisterFontResource(bytes.data(),bytes.size()));
    assert(font.SetResource(bytes.data()));assert(!font.SetResource(bytes.data()));
    nativeUnregisterFontResource(bytes.data());
    bytes.clear();bytes.shrink_to_fit();
    for(unsigned code=0;code<65536;++code)assert(font.GetGlyphIndex(uint16_t(code))==reference.glyphIndex(uint16_t(code)));
    assert(font.GetWidth()==reference.width && font.GetHeight()==reference.height);
    nw4r::ut::Glyph gpuGlyph; font.GetGlyph(&gpuGlyph,'A');auto gpuAddress=Native::gpuResources().address(gpuGlyph.pTexture);assert(Native::gpuResources().resolve(gpuAddress,32));
font.RemoveResource();bool removed=false;try{Native::gpuResources().resolve(gpuAddress,32);}catch(const std::exception&){removed=true;}assert(removed);font.RemoveResource();assert(font.IsManaging(nullptr));
    unsigned char invalid[16]={};assert(nativeRegisterFontResource(invalid,sizeof(invalid)));
    assert(!font.SetResource(invalid));assert(font.IsManaging(nullptr));nativeUnregisterFontResource(invalid);
    std::puts("PASS bounded ResFont attachment, malformed resource rejection, independent ownership and repeated removal");
}

