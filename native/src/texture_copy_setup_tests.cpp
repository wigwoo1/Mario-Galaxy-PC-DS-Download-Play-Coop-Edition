#include <revolution/gx.h>
#include <Native/TextureCopyState.hpp>
#include <cassert>
#include <cstdio>
unsigned format(u32 command){return ((command>>4)&7)|(((command>>3)&1)<<3);}
int main(){
 GXSetTexCopySrc(17,23,608,448);auto state=Native::textureCopyState();
 assert(state.cpTexSrc==(0x49000000|17|23u<<10));assert(state.cpTexSize==(0x4a000000|607|447u<<10));
 GXSetTexCopyDst(608,448,GX_TF_RGB565,0);state=Native::textureCopyState();assert(state.cpTexStride==(0x4d000000|152));assert(format(state.cpTex)==4&&!(state.cpTex&(1u<<9))&&!state.cpTexZ&&((state.cpTex>>15)&3)==2);
 GXSetTexCopyDst(5,7,GX_TF_RGBA8,1);state=Native::textureCopyState();assert(state.cpTexStride==(0x4d000000|4)&&format(state.cpTex)==6&&(state.cpTex&(1u<<9)));
 GXSetTexCopyDst(8,8,GX_TF_I4,0);state=Native::textureCopyState();assert(state.cpTexStride==(0x4d000000|1)&&format(state.cpTex)==0&&((state.cpTex>>15)&3)==3);
 GXSetTexCopyDst(5,7,GX_TF_Z16,0);state=Native::textureCopyState();assert(state.cpTexZ&&format(state.cpTex)==11&&state.cpTexStride==(0x4d000000|2));
 GXSetTexCopyDst(5,7,GX_TF_RGB565,0);state=Native::textureCopyState();assert(!state.cpTexZ&&format(state.cpTex)==4&&((state.cpTex>>15)&3)==2);
 std::puts("PASS original texture-copy source, RGB565/RGBA/intensity/depth stride and format transitions");
}
