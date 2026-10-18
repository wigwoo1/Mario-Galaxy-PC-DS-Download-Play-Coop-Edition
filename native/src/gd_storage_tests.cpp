#include <revolution/gx/GXTexture.h>
#include <revolution/gx/GXTypes.h>
#include <revolution/gx/GXGeometry.h>
#include <revolution/gd/GDBase.h>
#include <revolution/gx/GXVert.h>
#include <cassert>
#include <cstdio>
#include <stdexcept>

int main() {
    u8 bytes[65] = {};
    bytes[64] = 0xab;
    GDLObj dl;
    GDInitGDLObj(&dl, bytes, 64);
    GDSetCurrent(&dl);
    GDWriteBPCmd(0x94123456);
    assert(bytes[0] == 0x61 && bytes[1] == 0x94 && bytes[2] == 0x12 &&
           bytes[3] == 0x34 && bytes[4] == 0x56);
    GDPadCurr32();
    assert(GDGetCurrOffset() == 32);
    for (unsigned i = 5; i < 32; ++i) assert(bytes[i] == 0);
    GDWriteCPCmd(0xa0, 0x01020304);
    assert(bytes[32] == 8 && bytes[33] == 0xa0 && bytes[34] == 1 && bytes[37] == 4);
    GDPadCurr32();
    GDFlushCurrToMem();
    assert(GDGetCurrOffset() == 64);
    bool overflow = false;
    try { GDWrite_u32(0xffffffff); } catch (const std::length_error&) { overflow = true; }
    assert(overflow && GDGetCurrOffset() == 64 && bytes[64] == 0xab);
    u8 small[9] = {};
    small[8] = 0xcd;
    GDInitGDLObj(&dl, small, 8);
    GDSetCurrent(&dl);
    GDWrite_u8(7);
    overflow = false;
    try { GDPadCurr32(); } catch (const std::length_error&) { overflow = true; }
    assert(overflow && GDGetCurrOffset() == 1 && small[8] == 0xcd);
    u8 invalidate[2]={0,0xad};
    GDInitGDLObj(&dl,invalidate,1);GDSetCurrent(&dl);
    GXInvalidateVtxCache();assert(GDGetCurrOffset()==1 && invalidate[0]==0x48 && invalidate[1]==0xad);
    overflow=false;try{GXInvalidateVtxCache();}catch(const std::length_error&){overflow=true;}
    assert(overflow && GDGetCurrOffset()==1 && invalidate[1]==0xad);
    for(unsigned e=0;e<3;++e)for(unsigned o=0;o<4;++o){GXTexRegion region;std::memset(&region,0xa5,sizeof(region));GXInitTexCacheRegion(&region,1,0x20000,GXTexCacheSize(e),0x60000,GXTexCacheSize(o));GXTexRegionInt state;std::memcpy(&state,&region,sizeof(state));assert(state.image1==(0x1000|((e+3)<<15)|((e+3)<<18)));unsigned ow=o==3?0:o+3;assert(state.image2==(0x3000|(ow<<15)|(ow<<18)));assert(state.sizeEven==0xa5a5 && state.sizeOdd==0xa5a5 && state.is32bMipmap==1 && state.isCached==1);}
    u8 displayBytes[32]={};GDInitGDLObj(&dl,displayBytes,sizeof(displayBytes));GDSetCurrent(&dl);
    GXPosition2u16(0x1234,0x5678);GXTexCoord2u8(1,0);
    GDWrite_u8(0x61);GDWrite_u32(0x5800000f);
    const u8 displayExpected[]={0x12,0x34,0x56,0x78,1,0,0x61,0x58,0,0,0x0f};
    assert(GDGetCurrOffset()==sizeof(displayExpected));
    for(unsigned i=0;i<sizeof(displayExpected);i++)assert(displayBytes[i]==displayExpected[i]);
    GDSetCurrent(nullptr);
    bool missing = false;
    try { GDWrite_u8(1); } catch (const std::logic_error&) { missing = true; }
    assert(missing);
    bool invalid = false;
    try { GDInitGDLObj(&dl, nullptr, 64); } catch (const std::invalid_argument&) { invalid = true; }
    assert(invalid);
    std::puts("GD storage: big-endian BP/CP encoding, alignment, overflow and missing storage passed");
}



