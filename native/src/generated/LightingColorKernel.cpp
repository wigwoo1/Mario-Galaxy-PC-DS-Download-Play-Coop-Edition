#include <revolution/gx/GXLighting.h>
#include <revolution/gd/GDBase.h>
#include <private/xf_mem.h>
#include <revolution/gx/GXRegs.h>
#undef FAST_GPFLAGSET
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_xf_mem.h>
#undef gx
namespace {struct ColorCache{u32 ambColor[2]{},matColor[2]{};};ColorCache colors;auto* gx=&colors;
u32 pack(GXColor c){return (u32(c.r)<<24)|(u32(c.g)<<16)|(u32(c.b)<<8)|c.a;}}
void OriginalGXSetChanAmbColor(GXChannelID chan, GXColor amb_color) {
    u32 reg = 0, rgb, colIdx;
    switch (chan) {
    case GX_COLOR0:
        reg = gx->ambColor[GX_COLOR0];
        rgb = (pack(amb_color) >> 8);
        SC_XF_AMBIENT0_F_SET_RGB(reg, rgb);
        colIdx = 0;
        break;
    case GX_COLOR1:
        reg = gx->ambColor[GX_COLOR1];
        rgb = (pack(amb_color) >> 8);
        SC_XF_AMBIENT1_F_SET_RGB(reg, rgb);
        colIdx = 1;
        break;
    case GX_ALPHA0:
        reg = gx->ambColor[GX_COLOR0];
        SC_XF_AMBIENT0_F_SET_ALPHA(reg, amb_color.a);
        colIdx = 0;
        break;
    case GX_ALPHA1:
        reg = gx->ambColor[GX_COLOR1];
        SC_XF_AMBIENT1_F_SET_ALPHA(reg, amb_color.a);
        colIdx = 1;
        break;
    case GX_COLOR0A0:
        SC_XF_AMBIENT0_F_SET_RGBA(reg, pack(amb_color));
        colIdx = 0;
        break;
    case GX_COLOR1A1:
        SC_XF_AMBIENT1_F_SET_RGBA(reg, pack(amb_color));
        colIdx = 1;
        break;
    default:
        return;
    }

    GDOverflowCheck(9);GDWriteXFCmd(0x100a+colIdx,reg);
    gx->ambColor[colIdx] = reg;
}
void OriginalGXSetChanMatColor(GXChannelID chan, GXColor mat_color) {
    u32 reg = 0, rgb, colIdx;

    switch (chan) {
    case GX_COLOR0:
        reg = gx->matColor[GX_COLOR0];
        rgb = (pack(mat_color) >> 8);
        SC_XF_MATERIAL0_F_SET_RGB(reg, rgb);
        colIdx = 0;
        break;

    case GX_COLOR1:
        reg = gx->matColor[GX_COLOR1];
        rgb = (pack(mat_color) >> 8);
        SC_XF_MATERIAL1_F_SET_RGB(reg, rgb);
        colIdx = 1;
        break;

    case GX_ALPHA0:
        reg = gx->matColor[GX_COLOR0];
        SC_XF_MATERIAL0_F_SET_ALPHA(reg, mat_color.a);
        colIdx = 0;
        break;

    case GX_ALPHA1:
        reg = gx->matColor[GX_COLOR1];
        SC_XF_MATERIAL1_F_SET_ALPHA(reg, mat_color.a);
        colIdx = 1;
        break;

    case GX_COLOR0A0:
        SC_XF_MATERIAL0_F_SET_RGBA(reg, pack(mat_color));
        colIdx = 0;
        break;

    case GX_COLOR1A1:
        SC_XF_MATERIAL1_F_SET_RGBA(reg, pack(mat_color));
        colIdx = 1;
        break;

    default:
        return;
    }

    GDOverflowCheck(9);GDWriteXFCmd(0x100c+colIdx,reg);
    gx->matColor[colIdx] = reg;
}

