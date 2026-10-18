#include <Native/DiffuseLighting.hpp>
#include <revolution/gx/GXLighting.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
void OriginalGXSetChanAmbColor(GXChannelID,GXColor);void OriginalGXSetChanMatColor(GXChannelID,GXColor);
int main(){
 u8 bytes[90]{};GDLObj writer;GDInitGDLObj(&writer,bytes,90);GDSetCurrent(&writer);
 u8 reference[72];GDLObj original;GDInitGDLObj(&original,reference,72);GDSetCurrent(&original);
 OriginalGXSetChanAmbColor(GX_COLOR0A0,{13,71,149,227});OriginalGXSetChanAmbColor(GX_COLOR1A1,{19,31,53,79});
 OriginalGXSetChanAmbColor(GX_COLOR0,{1,2,3,0});OriginalGXSetChanAmbColor(GX_ALPHA1,{0,0,0,103});
 OriginalGXSetChanMatColor(GX_COLOR0A0,{17,29,43,61});OriginalGXSetChanMatColor(GX_COLOR1A1,{89,97,107,113});
 OriginalGXSetChanMatColor(GX_ALPHA0,{0,0,0,127});OriginalGXSetChanMatColor(GX_COLOR1,{131,137,139,0});GDSetCurrent(&writer);
 GXSetChanAmbColor(GX_COLOR0A0,{13,71,149,227});GXSetChanAmbColor(GX_COLOR1A1,{19,31,53,79});
 GXSetChanAmbColor(GX_COLOR0,{1,2,3,0});GXSetChanAmbColor(GX_ALPHA1,{0,0,0,103});
 GXSetChanMatColor(GX_COLOR0A0,{17,29,43,61});GXSetChanMatColor(GX_COLOR1A1,{89,97,107,113});
 GXSetChanMatColor(GX_ALPHA0,{0,0,0,127});GXSetChanMatColor(GX_COLOR1,{131,137,139,0});
 assert(GDGetCurrOffset()==72&&std::memcmp(reference,bytes,72)==0);
 Native::GxCommandState state;state.consume(bytes,GDGetCurrOffset());
 assert((state.lightingColor(0,true)==std::array<unsigned,4>{1,2,3,227}));assert((state.lightingColor(1,true)==std::array<unsigned,4>{19,31,53,103}));
 assert((state.lightingColor(0)==std::array<unsigned,4>{17,29,43,127}));assert((state.lightingColor(1)==std::array<unsigned,4>{131,137,139,113}));
 GDInitGDLObj(&writer,bytes,8);bool rejected=false;try{GXSetChanAmbColor(GX_COLOR0A0,{255,255,255,255});}catch(const std::exception&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
 GDInitGDLObj(&writer,bytes,90);GXSetChanAmbColor(GX_ALPHA0,{0,0,0,83});state.consume(bytes,GDGetCurrOffset());assert((state.lightingColor(0,true)==std::array<unsigned,4>{1,2,3,83}));
 GDInitGDLObj(&writer,bytes,90);GXSetChanAmbColor(GX_COLOR0A0,{128,128,128,128});GXSetChanMatColor(GX_COLOR0A0,{128,128,128,128});GDWriteXFCmd(0x100e,2);GDWriteXFCmd(0x1010,2);state.consume(bytes,GDGetCurrOffset());
 auto lit=Native::evaluateDiffuseLighting(state,0,{0,0,0},{0,0,1},{255,255,255,255});assert((lit==std::array<unsigned,4>{64,64,64,64}));
 GDSetCurrent(nullptr);std::puts("PASS original GX ambient/material channel color updates, native packing, atomic rejection and lighting output");
}

