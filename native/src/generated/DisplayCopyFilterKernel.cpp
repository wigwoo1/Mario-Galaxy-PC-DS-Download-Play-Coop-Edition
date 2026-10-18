#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <private/gen_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
void GXSetCopyClear(GXColor clear_clr, u32 clear_z) {
    GDOverflowCheck(15);
    u32 reg;
    reg = 0;
    SC_PE_COPY_CLEAR_COLOR_AR_SET_RED(reg, clear_clr.r);
    SC_PE_COPY_CLEAR_COLOR_AR_SET_ALPHA(reg, clear_clr.a);
    SC_PE_COPY_CLEAR_COLOR_AR_SET_RID(reg, 0x4F);
    GDWriteBPCmd(reg);

    reg = 0;
    SC_PE_COPY_CLEAR_COLOR_GB_SET_BLUE(reg, clear_clr.b);
    SC_PE_COPY_CLEAR_COLOR_GB_SET_GREEN(reg, clear_clr.g);
    SC_PE_COPY_CLEAR_COLOR_GB_SET_RID(reg, 0x50);
    GDWriteBPCmd(reg);

    reg = 0;
    SC_PE_COPY_CLEAR_Z_SET_DATA(reg, clear_z);
    SC_PE_COPY_CLEAR_Z_SET_RID(reg, 0x51);
    GDWriteBPCmd(reg);
    
}
void GXSetCopyFilter(GXBool aa, const u8 sample_pattern[12][2], GXBool vf, const u8 vfilter[7]) {
    GDOverflowCheck(30);
    u32 msLoc[4];
    u32 coeff0, coeff1;

    if (aa) {
        msLoc[0] = 0;
        SC_GEN_MSLOC_SET_XS0(msLoc[0], sample_pattern[0][0]);
        SC_GEN_MSLOC_SET_YS0(msLoc[0], sample_pattern[0][1]);
        SC_GEN_MSLOC_SET_XS1(msLoc[0], sample_pattern[1][0]);
        SC_GEN_MSLOC_SET_YS1(msLoc[0], sample_pattern[1][1]);
        SC_GEN_MSLOC_SET_XS2(msLoc[0], sample_pattern[2][0]);
        SC_GEN_MSLOC_SET_YS2(msLoc[0], sample_pattern[2][1]);
        SC_GEN_MSLOC_SET_RID(msLoc[0], 1);

        msLoc[1] = 0;
        SC_GEN_MSLOC_SET_XS0(msLoc[1], sample_pattern[3][0]);
        SC_GEN_MSLOC_SET_YS0(msLoc[1], sample_pattern[3][1]);
        SC_GEN_MSLOC_SET_XS1(msLoc[1], sample_pattern[4][0]);
        SC_GEN_MSLOC_SET_YS1(msLoc[1], sample_pattern[4][1]);
        SC_GEN_MSLOC_SET_XS2(msLoc[1], sample_pattern[5][0]);
        SC_GEN_MSLOC_SET_YS2(msLoc[1], sample_pattern[5][1]);
        SC_GEN_MSLOC_SET_RID(msLoc[1], 2);

        msLoc[2] = 0;
        SC_GEN_MSLOC_SET_XS0(msLoc[2], sample_pattern[6][0]);
        SC_GEN_MSLOC_SET_YS0(msLoc[2], sample_pattern[6][1]);
        SC_GEN_MSLOC_SET_XS1(msLoc[2], sample_pattern[7][0]);
        SC_GEN_MSLOC_SET_YS1(msLoc[2], sample_pattern[7][1]);
        SC_GEN_MSLOC_SET_XS2(msLoc[2], sample_pattern[8][0]);
        SC_GEN_MSLOC_SET_YS2(msLoc[2], sample_pattern[8][1]);
        SC_GEN_MSLOC_SET_RID(msLoc[2], 3);

        msLoc[3] = 0;
        SC_GEN_MSLOC_SET_XS0(msLoc[3], sample_pattern[9][0]);
        SC_GEN_MSLOC_SET_YS0(msLoc[3], sample_pattern[9][1]);
        SC_GEN_MSLOC_SET_XS1(msLoc[3], sample_pattern[10][0]);
        SC_GEN_MSLOC_SET_YS1(msLoc[3], sample_pattern[10][1]);
        SC_GEN_MSLOC_SET_XS2(msLoc[3], sample_pattern[11][0]);
        SC_GEN_MSLOC_SET_YS2(msLoc[3], sample_pattern[11][1]);
        SC_GEN_MSLOC_SET_RID(msLoc[3], 4);
    } else {
        msLoc[0] = GEN_MSLOC(6, 6, 6, 6, 6, 6, 1);
        msLoc[1] = GEN_MSLOC(6, 6, 6, 6, 6, 6, 2);
        msLoc[2] = GEN_MSLOC(6, 6, 6, 6, 6, 6, 3);
        msLoc[3] = GEN_MSLOC(6, 6, 6, 6, 6, 6, 4);
    }

    GDWriteBPCmd(msLoc[0]);
    GDWriteBPCmd(msLoc[1]);
    GDWriteBPCmd(msLoc[2]);
    GDWriteBPCmd(msLoc[3]);

    coeff0 = 0;
    SC_PE_COPY_VFILTER0_SET_RID(coeff0, 0x53);
    coeff1 = 0;
    SC_PE_COPY_VFILTER1_SET_RID(coeff1, 0x54);

    if (vf) {
        SC_PE_COPY_VFILTER0_SET_COEFF0(coeff0, vfilter[0]);
        SC_PE_COPY_VFILTER0_SET_COEFF1(coeff0, vfilter[1]);
        SC_PE_COPY_VFILTER0_SET_COEFF2(coeff0, vfilter[2]);
        SC_PE_COPY_VFILTER0_SET_COEFF3(coeff0, vfilter[3]);
        SC_PE_COPY_VFILTER1_SET_COEFF4(coeff1, vfilter[4]);
        SC_PE_COPY_VFILTER1_SET_COEFF5(coeff1, vfilter[5]);
        SC_PE_COPY_VFILTER1_SET_COEFF6(coeff1, vfilter[6]);
    } else {
        SC_PE_COPY_VFILTER0_SET_COEFF0(coeff0, 0);
        SC_PE_COPY_VFILTER0_SET_COEFF1(coeff0, 0);
        SC_PE_COPY_VFILTER0_SET_COEFF2(coeff0, 21);
        SC_PE_COPY_VFILTER0_SET_COEFF3(coeff0, 22);
        SC_PE_COPY_VFILTER1_SET_COEFF4(coeff1, 21);
        SC_PE_COPY_VFILTER1_SET_COEFF5(coeff1, 0);
        SC_PE_COPY_VFILTER1_SET_COEFF6(coeff1, 0);
    }

    GDWriteBPCmd(coeff0);
    GDWriteBPCmd(coeff1);
    
}

