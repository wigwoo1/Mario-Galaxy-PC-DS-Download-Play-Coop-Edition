#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <initializer_list>
u32 word(const u8*p){assert(p[0]==0x61);return u32(p[1])<<24|u32(p[2])<<16|u32(p[3])<<8|p[4];}
int main(){
 u8 bytes[32];GDLObj dl;GDInitGDLObj(&dl,bytes,32);GDSetCurrent(&dl);
 // near=1, far=2, start=0, end=1 gives A=2, normalized a=.5, B magnitude=1, shift=2.
 GXSetFog(GX_FOG_PERSP_LIN,0,1,1,2,GXColor{0x12,0x34,0x56,0x78});
 assert(GDGetCurrOffset()==25);
 assert(word(bytes)==0xee03f000&&word(bytes+5)==0xef80001e&&word(bytes+10)==0xf0000002&&word(bytes+15)==0xf1400000&&word(bytes+20)==0xf2123456);
 GDInitGDLObj(&dl,bytes,32);
 GXSetFog(GX_FOG_ORTHO_LIN,0,100,0,100,GXColor{0xab,0xcd,0xef,0});
 // Preserve the pinned original GX routine's two zero BP words in orthographic mode.
 assert(word(bytes)==0xee03f800&&word(bytes+5)==0&&word(bytes+10)==0&&word(bytes+15)==0xf1500000&&word(bytes+20)==0xf2abcdef);
 for(bool equalNear:{false,true}){
  GDInitGDLObj(&dl,bytes,32);
  GXSetFog(GX_FOG_PERSP_LIN,0,equalNear?1:0,1,equalNear?1:2,GXColor{});
  assert(word(bytes)==0xee000000&&word(bytes+5)==0xef40000f&&word(bytes+10)==0xf0000001&&word(bytes+15)==0xf1400000);
 }
 for(auto&b:bytes)b=0xa5;GDInitGDLObj(&dl,bytes,24);
 bool rejected=false;try{GXSetFog(GX_FOG_PERSP_LIN,0,1,1,2,GXColor{});}catch(const std::length_error&){rejected=true;}
 assert(rejected&&GDGetCurrOffset()==0);for(auto b:bytes)assert(b==0xa5);
 GDSetCurrent(nullptr);std::puts("PASS original GX fog: perspective, orthographic, degenerate bounds, RGB packing and atomic capacity rejection");
}
