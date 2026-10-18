#include <revolution/gx.h>
#undef gx
#include <Native/DisplayCopyState.hpp>
#include <revolution/gd/GDBase.h>
#include <cmath>
#include <stdexcept>
namespace Native {void resetTextureCopyInterlacing();void setTextureCopyClamp(bool,bool);}
namespace {Native::DisplayCopyState copyState;auto* gx=&copyState;}
namespace Native {const DisplayCopyState& displayCopyState(){return copyState;}}

#include <private/gen_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
static u32 __GXGetNumXfbLines(u32 efbHt, u32 iScale) {
    u32 count, realHt, iScaleD;

    count = (efbHt - 1) * 256;
    realHt = count / iScale + 1;

    iScaleD = iScale;
    if (iScaleD > 0x80 && iScaleD < 0x100) {
        while ((iScaleD & 0x01) == 0) {
            iScaleD >>= 1;
        }

        if ((efbHt % iScaleD) == 0) {
            ++realHt;
        }
    }

    if (realHt > 1024) {
        realHt = 1024;
    }

    return realHt;
}
u32 GXSetDispCopyYScale(f32 vscale) {
    GXBool enable;
    u32 iScale;
    u32 ht;
    u32 reg;

    if(!std::isfinite(vscale)||vscale<=0 || 256.0f/vscale>4294967040.0f || ((u32)(256.0f/vscale)&0x1ff)==0)throw std::invalid_argument("Invalid display-copy scale"); GDOverflowCheck(5);
    iScale = (u32)(256.0f / vscale) & 0x1ff;
    enable = (iScale != 0x100);

    reg = 0;
    SC_PE_COPY_SCALE_SET_SCALE(reg, iScale);
    SC_PE_COPY_SCALE_SET_RID(reg, 0x4E);
    GDWriteBPCmd(reg);
    

    SC_PE_COPY_CMD_SET_VERTICAL_SCALE(gx->cpDisp, enable);
    ht = PE_COPY_SRC_SIZE_GET_Y(gx->cpDispSize) + 1;

    return (__GXGetNumXfbLines(ht, iScale));
}
void GXSetCopyClamp(GXFBClamp clamp) {
    GXBool clmpB, clmpT;

    clmpT = ((clamp & GX_CLAMP_TOP) == GX_CLAMP_TOP);
    clmpB = ((clamp & GX_CLAMP_BOTTOM) == GX_CLAMP_BOTTOM);

    SC_PE_COPY_CMD_SET_CLAMP_TOP(gx->cpDisp, clmpT);
    SC_PE_COPY_CMD_SET_CLAMP_BOTTOM(gx->cpDisp, clmpB);

    Native::setTextureCopyClamp(clmpT,clmpB);

}
void GXSetDispCopySrc(u16 left, u16 top, u16 wd, u16 ht) {
    gx->cpDispSrc = 0;
    SC_PE_COPY_SRC_ADDR_SET_X(gx->cpDispSrc, left);
    SC_PE_COPY_SRC_ADDR_SET_Y(gx->cpDispSrc, top);
    SC_PE_COPY_SRC_ADDR_SET_RID(gx->cpDispSrc, 0x49);

    gx->cpDispSize = 0;
    SC_PE_COPY_SRC_SIZE_SET_X(gx->cpDispSize, (wd - 1));
    SC_PE_COPY_SRC_SIZE_SET_Y(gx->cpDispSize, (ht - 1));
    SC_PE_COPY_SRC_SIZE_SET_RID(gx->cpDispSize, 0x4A);
}
void GXSetDispCopyDst(u16 wd, u16 ht) {
    u16 stride;

    stride = (u16)(wd * VI_DISPLAY_PIX_SZ);
    gx->cpDispStride = 0;
    SC_PE_COPY_DST_STRIDE_SET_STRIDE(gx->cpDispStride, (stride >> 5));
    SC_PE_COPY_DST_STRIDE_SET_RID(gx->cpDispStride, 0x4D);
}
void GXSetDispCopyFrame2Field(GXCopyMode mode) {
    SC_PE_COPY_CMD_SET_INTERLACED(gx->cpDisp, mode);
    Native::resetTextureCopyInterlacing();
}
void GXSetDispCopyGamma(GXGamma gamma) {
    SC_PE_COPY_CMD_SET_GAMMA(gx->cpDisp, gamma);
}

