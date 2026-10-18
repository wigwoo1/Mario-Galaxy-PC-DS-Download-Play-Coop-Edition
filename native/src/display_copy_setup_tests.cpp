#include <Native/DisplayCopyState.hpp>
#include <Native/TextureCopyState.hpp>
#include <Native/GxCommandState.hpp>
#include <revolution/gd/GDBase.h>
#include <revolution/gx.h>
#include <cassert>
#include <cstdio>
int main(){
 GXSetDispCopySrc(17,23,608,448);GXSetDispCopyDst(608,448);
 auto a=Native::displayCopyState();assert(a.cpDispSrc==(0x49000000u|17|23u<<10));assert(a.cpDispSize==(0x4a000000u|607|447u<<10));assert(a.cpDispStride==0x4d000026);
 GXSetDispCopyGamma(GX_GM_1_7);GXSetDispCopyFrame2Field(GX_COPY_INTLC_EVEN);
 auto b=Native::displayCopyState();assert(((b.cpDisp>>7)&3)==1&&((b.cpDisp>>12)&3)==2);
 GXSetDispCopyGamma(GX_GM_1_0);GXSetDispCopyFrame2Field(GX_COPY_PROGRESSIVE);
 assert(((Native::displayCopyState().cpDisp>>7)&3)==0&&((Native::displayCopyState().cpDisp>>12)&3)==0);
 u8 bytes[5]{};GDLObj writer;GDInitGDLObj(&writer,bytes,5);GDSetCurrent(&writer);
 assert(GXSetDispCopyYScale(1.0f)==448);Native::GxCommandState registers;registers.consume(bytes,5);
 assert(registers.bp(0x4e)==256 && !(Native::displayCopyState().cpDisp&(1u<<10)));
 GXSetDispCopySrc(0,0,640,384);GDInitGDLObj(&writer,bytes,5);
 assert(GXSetDispCopyYScale(2.0f)==767);registers.consume(bytes,5);
 assert(registers.bp(0x4e)==128 && (Native::displayCopyState().cpDisp&(1u<<10)));
 for(unsigned clamp=0;clamp<4;++clamp){GXSetCopyClamp(GXFBClamp(clamp));assert((Native::displayCopyState().cpDisp&3)==clamp && (Native::textureCopyState().cpTex&3)==clamp);}
 auto before=Native::displayCopyState().cpDisp;GDInitGDLObj(&writer,bytes,4);
 bool rejected=false;try{GXSetDispCopyYScale(1.0f);}catch(const std::exception&){rejected=true;}
 assert(rejected && GDGetCurrOffset()==0 && Native::displayCopyState().cpDisp==before);
 GDInitGDLObj(&writer,bytes,5);rejected=false;try{GXSetDispCopyYScale(0);}catch(const std::exception&){rejected=true;}
 assert(rejected && GDGetCurrOffset()==0);GDSetCurrent(nullptr);
 std::puts("PASS original display copy setup: source rectangle, XFB stride, gamma, field transitions, Y scale commands, shared clamp and atomic rejection");
}

