#include <Native/GxCommandState.hpp>
#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
#include <stdexcept>
int main(){
 u8 bytes[32]{};GDLObj dl;Native::GxCommandState state;
 for(unsigned coord=0;coord<8;++coord){
  GDInitGDLObj(&dl,bytes,sizeof(bytes));GDSetCurrent(&dl);GDWriteBPCmd(((0x30+coord*2)<<24)|0x034567);state.consume(bytes,5);
  for(unsigned line=0;line<2;++line)for(unsigned point=0;point<2;++point){
   GDInitGDLObj(&dl,bytes,sizeof(bytes));GXEnableTexOffsets(GXTexCoordID(coord),GXBool(line),GXBool(point));assert(GDGetCurrOffset()==10);
   state.consume(bytes,10);assert(state.bp(0x30+coord*2)==(0x034567|(line<<18)|(point<<19)));
  }
 }
 for(unsigned stage=0;stage<16;++stage)for(unsigned indirect=0;indirect<4;++indirect)
 for(unsigned bias=0;bias<2;++bias)for(unsigned replace=0;replace<2;++replace){
  GDInitGDLObj(&dl,bytes,sizeof(bytes));GXSetTevIndWarp(GXTevStageID(stage),GXIndTexStageID(indirect),GXBool(bias),GXBool(replace),GX_ITM_1);assert(GDGetCurrOffset()==5);
  // Independent BP bitfield reference from the original SDK specification.
  const u32 expected=indirect|(bias?7u<<4:0)|(2u<<9)|(replace?(6u<<13)|(6u<<16):0);
  state.consume(bytes,5);assert(state.bp(0x10+stage)==expected);
 }
 for(unsigned capacity=1;capacity<10;++capacity){
  for(auto&b:bytes)b=0xa5;GDInitGDLObj(&dl,bytes,capacity);bool rejected=false;
  try{GXEnableTexOffsets(GX_TEXCOORD0,GX_TRUE,GX_TRUE);}catch(const std::length_error&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);for(auto b:bytes)assert(b==0xa5);
 }
 GDInitGDLObj(&dl,bytes,sizeof(bytes));bool rejected=false;try{GXEnableTexOffsets(GX_TEXCOORD_NULL,GX_TRUE,GX_TRUE);}catch(const std::invalid_argument&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
 rejected=false;try{GXSetTevIndWarp(GX_TEVSTAGE0,GX_INDTEXSTAGE0,GXBool(2),GX_FALSE,GX_ITM_1);}catch(const std::invalid_argument&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
 GDSetCurrent(nullptr);puts("PASS original indirect warp fields and masked texture offsets: all stages/coords/flags, preserved size bits, bounded atomic output");
}

