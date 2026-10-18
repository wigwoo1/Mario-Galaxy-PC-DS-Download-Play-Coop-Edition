#include <JSystem/J3DGraphBase/J3DGD.hpp>
#include <revolution/gd/GDTev.h>
void J3DGDSetTevKColor(GXTevKColorID reg, GXColor color) {
    u32 regRA;
    u32 regBG;

    regRA = BP_TEV_COLOR_REG_RA(color.r, color.a, 1, 0xE0 + reg * 2);
    regBG = BP_TEV_COLOR_REG_BG(color.b, color.g, 1, 0xE1 + reg * 2);

    GDOverflowCheck(0xa);
    J3DGDWriteBPCmd(regRA);
    J3DGDWriteBPCmd(regBG);
}

void J3DGDSetTevColorS10(GXTevRegID reg, GXColorS10 color) {
    u32 regRA;
    u32 regBG;

    regRA = BP_TEV_COLOR_REG_RA(color.r & 0x7FF, color.a & 0x7FF, 0, 0xE0 + reg * 2);
    regBG = BP_TEV_COLOR_REG_BG(color.b & 0x7FF, color.g & 0x7FF, 0, 0xE1 + reg * 2);

    GDOverflowCheck(0x14);
    J3DGDWriteBPCmd(regRA);
    J3DGDWriteBPCmd(regBG);
    J3DGDWriteBPCmd(regBG);
    J3DGDWriteBPCmd(regBG);
}

