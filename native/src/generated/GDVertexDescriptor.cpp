#include "revolution/gd.h"
#include "revolution/gx.h"
#include "revolution/os.h"

void GDSetVtxDescv(const GXVtxDescList* attrPtr) {
    u32 nnorms = 0;
    u32 ncols = 0;
    u32 ntexs = 0;

    u32 pnMtxIdx = GX_NONE;
    u32 txMtxIdxMask = 0;
    u32 posn = GX_DIRECT;
    u32 norm = GX_NONE;
    u32 col0 = GX_NONE;
    u32 col1 = GX_NONE;
    u32 tex0 = GX_NONE;
    u32 tex1 = GX_NONE;
    u32 tex2 = GX_NONE;
    u32 tex3 = GX_NONE;
    u32 tex4 = GX_NONE;
    u32 tex5 = GX_NONE;
    u32 tex6 = GX_NONE;
    u32 tex7 = GX_NONE;

    while (attrPtr->attr != GX_VA_NULL) {
        switch (attrPtr->attr) {
        case GX_VA_PNMTXIDX:
            pnMtxIdx = attrPtr->type;
            break;

        case GX_VA_TEX0MTXIDX:
            txMtxIdxMask = (txMtxIdxMask & ~1) | (attrPtr->type << 0);
            break;
        case GX_VA_TEX1MTXIDX:
            txMtxIdxMask = (txMtxIdxMask & ~2) | (attrPtr->type << 1);
            break;
        case GX_VA_TEX2MTXIDX:
            txMtxIdxMask = (txMtxIdxMask & ~4) | (attrPtr->type << 2);
            break;
        case GX_VA_TEX3MTXIDX:
            txMtxIdxMask = (txMtxIdxMask & ~8) | (attrPtr->type << 3);
            break;
        case GX_VA_TEX4MTXIDX:
            txMtxIdxMask = (txMtxIdxMask & ~16) | (attrPtr->type << 4);
            break;
        case GX_VA_TEX5MTXIDX:
            txMtxIdxMask = (txMtxIdxMask & ~32) | (attrPtr->type << 5);
            break;
        case GX_VA_TEX6MTXIDX:
            txMtxIdxMask = (txMtxIdxMask & ~64) | (attrPtr->type << 6);
            break;
        case GX_VA_TEX7MTXIDX:
            txMtxIdxMask = (txMtxIdxMask & ~128) | (attrPtr->type << 7);
            break;

        case GX_VA_POS:
            posn = attrPtr->type;
            break;

        case GX_VA_NRM:
            if (attrPtr->type != GX_NONE) {
                norm = attrPtr->type;
                nnorms = 1;
            }
            break;
        case GX_VA_NBT:
            if (attrPtr->type != GX_NONE) {
                norm = attrPtr->type;
                nnorms = 2;
            }
            break;

        case GX_VA_CLR0:
            col0 = attrPtr->type;
            ncols += (col0 != GX_NONE);
            break;
        case GX_VA_CLR1:
            col1 = attrPtr->type;
            ncols += (col1 != GX_NONE);
            break;

        case GX_VA_TEX0:
            tex0 = attrPtr->type;
            ntexs += (tex0 != GX_NONE);
            break;
        case GX_VA_TEX1:
            tex1 = attrPtr->type;
            ntexs += (tex1 != GX_NONE);
            break;
        case GX_VA_TEX2:
            tex2 = attrPtr->type;
            ntexs += (tex2 != GX_NONE);
            break;
        case GX_VA_TEX3:
            tex3 = attrPtr->type;
            ntexs += (tex3 != GX_NONE);
            break;
        case GX_VA_TEX4:
            tex4 = attrPtr->type;
            ntexs += (tex4 != GX_NONE);
            break;
        case GX_VA_TEX5:
            tex5 = attrPtr->type;
            ntexs += (tex5 != GX_NONE);
            break;
        case GX_VA_TEX6:
            tex6 = attrPtr->type;
            ntexs += (tex6 != GX_NONE);
            break;
        case GX_VA_TEX7:
            tex7 = attrPtr->type;
            ntexs += (tex7 != GX_NONE);
            break;
        default:
            break;
        }
        attrPtr++;
    }

    GDWriteCPCmd(CP_VCD_LO_ID, CP_VCD_REG_LO_PS(pnMtxIdx, txMtxIdxMask, posn, norm, col0, col1));
    GDWriteCPCmd(CP_VCD_HI_ID, CP_VCD_REG_HI(tex0, tex1, tex2, tex3, tex4, tex5, tex6, tex7));
    GDWriteXFCmd(XF_INVTXSPEC_ID, XF_INVTXSPEC(ncols, nnorms, ntexs));
}


