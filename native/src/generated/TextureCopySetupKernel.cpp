#include <revolution/gx.h>
#undef gx
#include <Native/TextureCopyState.hpp>
namespace {Native::TextureCopyState copyState;auto* gx=&copyState;}
namespace Native {const TextureCopyState& textureCopyState(){return copyState;} void resetTextureCopyInterlacing(){copyState.cpTex&=~(3u<<12);} void setTextureCopyClamp(bool top,bool bottom){copyState.cpTex=(copyState.cpTex&~3u)|u32(top)|(u32(bottom)<<1);}}
static void __GXGetTexTileShift(GXTexFmt,u32*,u32*);
#include <private/gen_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
void __GetImageTileCount(GXTexFmt fmt, u16 wd, u16 ht, u32* rowTiles, u32* colTiles, u32* cmpTiles) {
    u32 texRowShift, texColShift;

    __GXGetTexTileShift(fmt, &texRowShift, &texColShift);

    if (wd == 0)
        wd = 1;
    if (ht == 0)
        ht = 1;
    *rowTiles = (u32)((wd + (1 << texRowShift) - 1) >> texRowShift);
    *colTiles = (u32)((ht + (1 << texColShift) - 1) >> texColShift);
    *cmpTiles = (fmt == GX_TF_RGBA8 || fmt == GX_TF_Z24X8) ? 2U : 1U;

    return;
}
static void __GXGetTexTileShift(GXTexFmt fmt, u32* rowTileS, u32* colTileS) {
    switch (fmt) {
    case GX_TF_C4:
    case GX_TF_I4:
    case GX_TF_CMPR:
    case GX_CTF_R4:
    case GX_CTF_Z4:
        *rowTileS = 3;
        *colTileS = 3;
        break;

    case GX_TF_C8:
    case GX_TF_I8:
    case GX_TF_IA4:
    case GX_TF_A8:
    case GX_TF_Z8:
    case GX_CTF_RA4:
    case GX_CTF_R8:
    case GX_CTF_G8:
    case GX_CTF_B8:
    case GX_CTF_Z8M:
    case GX_CTF_Z8L:
        *rowTileS = 3;
        *colTileS = 2;
        break;

    // 16-bit+ formats:
    case GX_TF_C14X2:
    case GX_TF_IA8:
    case GX_TF_RGB565:
    case GX_TF_RGB5A3:
    case GX_TF_RGBA8:
    case GX_TF_Z16:
    case GX_TF_Z24X8:
    case GX_CTF_RA8:
    case GX_CTF_RG8:
    case GX_CTF_GB8:
    case GX_CTF_Z16L:
        *rowTileS = 2;
        *colTileS = 2;
        break;
    default:
        *rowTileS = *colTileS = 0;
        break;
    }
    return;
}
void GXSetTexCopySrc(u16 left, u16 top, u16 wd, u16 ht) {
    gx->cpTexSrc = 0;
    SC_PE_COPY_SRC_ADDR_SET_X(gx->cpTexSrc, left);
    SC_PE_COPY_SRC_ADDR_SET_Y(gx->cpTexSrc, top);
    SC_PE_COPY_SRC_ADDR_SET_RID(gx->cpTexSrc, 0x49);

    gx->cpTexSize = 0;
    SC_PE_COPY_SRC_SIZE_SET_X(gx->cpTexSize, (wd - 1));
    SC_PE_COPY_SRC_SIZE_SET_Y(gx->cpTexSize, (ht - 1));
    SC_PE_COPY_SRC_SIZE_SET_RID(gx->cpTexSize, 0x4A);
}
void GXSetTexCopyDst(u16 wd, u16 ht, GXTexFmt fmt, GXBool mipmap) {
    u32 rowTiles, colTiles, cmpTiles;
    u32 peTexFmt;
    u32 peTexFmtH;

    gx->cpTexZ = false;
    peTexFmt = (u32)(fmt & 0xf);

    if (fmt == GX_TF_Z16)
        peTexFmt = 0xB;

    switch (fmt) {
    case GX_TF_I4:
    case GX_TF_I8:
    case GX_TF_IA4:
    case GX_TF_IA8:
    case GX_CTF_YUVA8:
        SC_PE_COPY_CMD_SET_CCV(gx->cpTex, 3);
        break;
    default:
        SC_PE_COPY_CMD_SET_CCV(gx->cpTex, 2);
        break;
    }

    gx->cpTexZ = ((fmt & _GX_TF_ZTF) == _GX_TF_ZTF);

    peTexFmtH = ((peTexFmt & 0x8) >> 3);
    SC_PE_COPY_CMD_SET_TEX_FORMATH(gx->cpTex, peTexFmtH);
    peTexFmt = (peTexFmt & 0x7);

    __GetImageTileCount(fmt, wd, ht, &rowTiles, &colTiles, &cmpTiles);

    gx->cpTexStride = 0;
    SC_PE_COPY_DST_STRIDE_SET_STRIDE(gx->cpTexStride, rowTiles * cmpTiles);
    SC_PE_COPY_DST_STRIDE_SET_RID(gx->cpTexStride, 0x4D);
    SC_PE_COPY_CMD_SET_MIP_MAP_FILTER(gx->cpTex, mipmap);
    SC_PE_COPY_CMD_SET_TEX_FORMAT(gx->cpTex, peTexFmt);
}

