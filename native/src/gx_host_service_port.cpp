#include <revolution/gx/GXFrameBuf.h>
#include <revolution/gx/GXPixel.h>
#include <revolution/gx/GXCpu2Efb.h>
#include <revolution/gx/GXFifo.h>
#include <revolution/gx/GXGeometry.h>
#include <revolution/gx/GXPerf.h>
#include <revolution/gx/GXTypes.h>
#include <revolution/gd/GDBase.h>
#include <Native/CommandSubmission.hpp>
#include <stdexcept>

extern "C" void GXSetFieldMask(GXBool oddMask,GXBool evenMask){
    if(unsigned(oddMask)>1 || unsigned(evenMask)>1)throw std::invalid_argument("GX field mask");
    GDOverflowCheck(5);
    GDWriteBPCmd(0x44000000u|(u32(oddMask)<<1)|u32(evenMask));
}

extern "C" void GXSetFieldMode(GXBool fieldMode,GXBool halfAspectRatio){
    if(unsigned(fieldMode)>1 || unsigned(halfAspectRatio)>1)throw std::invalid_argument("GX field mode");
    // Keep Petari's state bits and command stream. The host display is
    // progressive, but preserving these bits keeps the SDK state coherent.
    gx->lpSize=(gx->lpSize&~0x00400000u)|(u32(halfAspectRatio)<<22);
    GDOverflowCheck(10);
    GDWriteBPCmd(gx->lpSize);
    GDWriteBPCmd(0x68000000u|u32(fieldMode));
}

extern "C" void GXClearBoundingBox(){
    GDOverflowCheck(10);
    GDWriteBPCmd(0x550003ffu);
    GDWriteBPCmd(0x560003ffu);
}

extern "C" void GXPokeColorUpdate(GXBool enabled){
    if(unsigned(enabled)>1)throw std::invalid_argument("GX color update");
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfe000008u);
    GDWriteBPCmd(0x41000000u|(u32(enabled)<<3));
}

extern "C" void GXPokeAlphaUpdate(GXBool enabled){
    if(unsigned(enabled)>1)throw std::invalid_argument("GX alpha update");
    GDOverflowCheck(10);GDWriteBPCmd(0xfe000010u);GDWriteBPCmd(0x41000000u|(u32(enabled)<<4));
}

extern "C" void GXPokeDither(GXBool enabled){
    if(unsigned(enabled)>1)throw std::invalid_argument("GX dither");
    GDOverflowCheck(10);GDWriteBPCmd(0xfe000004u);GDWriteBPCmd(0x41000000u|(u32(enabled)<<2));
}

extern "C" void GXPokeBlendMode(GXBlendMode mode,GXBlendFactor source,GXBlendFactor destination,GXLogicOp logic){
    if(unsigned(mode)>3 || unsigned(source)>7 || unsigned(destination)>7 || unsigned(logic)>15)throw std::invalid_argument("GX blend mode");
    const u32 value=(unsigned(mode)&1)|(u32(mode==GX_BM_LOGIC)<<1)|(unsigned(destination)<<5)|(unsigned(source)<<8)|(u32(mode==GX_BM_SUBTRACT)<<11)|(unsigned(logic)<<12);
    GDOverflowCheck(10);GDWriteBPCmd(0xfe00ffe3u);GDWriteBPCmd(0x41000000u|value);
}

extern "C" void GXPokeAlphaMode(GXCompare function,u8 threshold){
    if(unsigned(function)>7)throw std::invalid_argument("GX alpha comparison");
    GDOverflowCheck(10);GDWriteBPCmd(0xfe0000ffu);GDWriteBPCmd(0x43000000u|(u32(function)<<8)|threshold);
}

extern "C" void GXPokeDstAlpha(GXBool enabled,u8 alpha){
    if(unsigned(enabled)>1)throw std::invalid_argument("GX destination alpha enable");
    GDOverflowCheck(10);GDWriteBPCmd(0xfe0001ffu);GDWriteBPCmd(0x42000000u|u32(alpha)|(u32(enabled)<<8));
}

extern "C" void GXPokeZMode(GXBool compare,GXCompare function,GXBool update){
    if(unsigned(compare)>1 || unsigned(function)>7 || unsigned(update)>1)throw std::invalid_argument("GX depth mode");
    GDOverflowCheck(10);GDWriteBPCmd(0xfe00001fu);GDWriteBPCmd(0x40000000u|u32(compare)|(u32(function)<<1)|(u32(update)<<4));
}

extern "C" void GXGetGPStatus(GXBool* overHi,GXBool* underLow,GXBool* readIdle,GXBool* commandIdle,GXBool* breakpoint){
    if(!overHi||!underLow||!readIdle||!commandIdle||!breakpoint)throw std::invalid_argument("GX status output");
    // The host backend has bounded CPU command buffers and executes submitted
    // batches synchronously; it has no Wii CP watermark or break-point unit.
    const GXBool pending=Native::gxCommandCompletionPending()?GX_TRUE:GX_FALSE;
    *overHi=GX_FALSE;*underLow=GX_FALSE;*readIdle=*commandIdle=GXBool(!pending);*breakpoint=GX_FALSE;
}

// The Wii CP FIFO breakpoint only throttles its asynchronous hardware reader.
// Native command submission consumes each bounded display list synchronously,
// and draw-sync tokens are completed at their decoded position by the backend.
// Therefore these hardware-only scheduling hints intentionally emit no GX
// commands and do not stand in for draw-sync completion.
extern "C" void GXEnableBreakPt(void*){}
extern "C" void GXDisableBreakPt(){}

// Petari's GX initializer calls this while constructing the initial display
// list. Preserve the actual general-mode BP register command in the host FIFO.
extern "C" void __GXSetGenMode(){
    GDWriteBPCmd(gx->genMode);
    gx->bpSentNot=GX_FALSE;
}

// Fixed-function XF/RAS counter registers have no direct D3D11 equivalent.
// Retain the requested selector state for compatibility, but report zero
// readings until a meaningful host-side metric mapping is implemented.
extern "C" void GXSetGPMetric(GXPerf0 perf0,GXPerf1 perf1){
    gx->perf0=perf0;
    gx->perf1=perf1;
    gx->bpSentNot=GX_FALSE;
}
extern "C" void GXClearGPMetric(){gx->bpSentNot=GX_FALSE;}
extern "C" void GXReadXfRasMetric(u32* xfWaitIn,u32* xfWaitOut,u32* rasBusy,u32* clocks){
    if(!xfWaitIn||!xfWaitOut||!rasBusy||!clocks)throw std::invalid_argument("GX metric output");
    *xfWaitIn=*xfWaitOut=*rasBusy=*clocks=0;
}
