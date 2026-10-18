#include <Native/GxCommandState.hpp>
#include <revolution/gx/GXPixel.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
int main(){u8 bytes[10]{};GDLObj writer;GDInitGDLObj(&writer,bytes,10);GDSetCurrent(&writer);
GXSetDstAlpha(GX_TRUE,173);assert(GDGetCurrOffset()==10);
const u8 expected[10]={0x61,0xfe,0,1,0xff,0x61,0x42,0,1,0xad};for(int i=0;i<10;i++)assert(bytes[i]==expected[i]);
 Native::GxCommandState state;state.consume(bytes,10);assert(state.bp(0x42)==0x1ad);
 assert(state.destinationAlpha().enabled&&state.destinationAlpha().alpha==173);
 GDInitGDLObj(&writer,bytes,10);GXSetDstAlpha(GX_FALSE,37);state.consume(bytes,10);assert(state.bp(0x42)==37);assert(!state.destinationAlpha().enabled&&state.destinationAlpha().alpha==37);
GDInitGDLObj(&writer,bytes,9);bool rejected=false;try{GXSetDstAlpha(GX_TRUE,255);}catch(const std::exception&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
GDInitGDLObj(&writer,bytes,10);rejected=false;try{GXSetDstAlpha(GXBool(2),255);}catch(const std::exception&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
GDSetCurrent(nullptr);std::puts("PASS destination alpha command bytes, enable/disable and atomic rejection");}
