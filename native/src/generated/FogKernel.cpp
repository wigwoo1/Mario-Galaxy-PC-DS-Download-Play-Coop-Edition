#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <cstring>
#include <private/tev_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_tev_reg.h>
void GXSetFog(GXFogType type, f32 startz, f32 endz, f32 nearz, f32 farz, GXColor color) {
    GDOverflowCheck(25);
    u32 fogclr = 0, fog0 = 0, fog1 = 0, fog2 = 0, fog3 = 0;
    f32 A, B, B_mant, C, a, c;
    u32 B_expn, b_m, b_s, a_hex, c_hex;
    u32 fsel, proj;
    u32 rgba;

    fsel = (u32)(type & 0x07);
    proj = (u32)((type >> 3) & 0x01);

    if (proj) {
        if ((farz == nearz) || (endz == startz)) {
            a = 0.0f;
            c = 0.0f;
        } else {
            A = 1.0F / (endz - startz);
            a = (farz - nearz) * A;
            c = (startz - nearz) * A;
        }
    } else {
        if ((farz == nearz) || (endz == startz)) {
            A = 0.0f;
            B = 0.5f;
            C = 0.0f;
        } else {
            A = (farz * nearz) / ((farz - nearz) * (endz - startz));
            B = farz / (farz - nearz);
            C = startz / (endz - startz);
        }

        B_mant = B;
        B_expn = 0;
        while (B_mant > 1.0) {
            B_mant /= 2;
            B_expn++;
        }
        while ((B_mant > 0) && (B_mant < 0.5)) {
            B_mant *= 2;
            B_expn--;
        }

        a = A / (1 << (B_expn + 1));
        b_m = (u32)(B_mant * 8388638);
        b_s = B_expn + 1;
        c = C;

        SC_TEV_FOG_PARAM_1_SET_B_MAG(fog1, b_m);
        SC_TEV_FOG_PARAM_1_SET_RID(fog1, 0xEF);

        SC_TEV_FOG_PARAM_2_SET_B_SHF(fog2, b_s);
        SC_TEV_FOG_PARAM_2_SET_RID(fog2, 0xF0);
    }

    std::memcpy(&a_hex,&a,4);
    std::memcpy(&c_hex,&c,4);

    SC_TEV_FOG_PARAM_0_SET_A_MANT(fog0, ((a_hex >> 12) & 0x7ff));
    SC_TEV_FOG_PARAM_0_SET_A_EXPN(fog0, ((a_hex >> 23) & 0x0ff));
    SC_TEV_FOG_PARAM_0_SET_A_SIGN(fog0, (a_hex >> 31));
    SC_TEV_FOG_PARAM_0_SET_RID(fog0, 0xEE);

    SC_TEV_FOG_PARAM_3_SET_C_MANT(fog3, ((c_hex >> 12) & 0x7ff));
    SC_TEV_FOG_PARAM_3_SET_C_EXPN(fog3, ((c_hex >> 23) & 0x0ff));
    SC_TEV_FOG_PARAM_3_SET_C_SIGN(fog3, (c_hex >> 31));
    SC_TEV_FOG_PARAM_3_SET_PROJ(fog3, proj);
    SC_TEV_FOG_PARAM_3_SET_FSEL(fog3, fsel);
    SC_TEV_FOG_PARAM_3_SET_RID(fog3, 0xF1);

    rgba = u32(color.r)<<24|u32(color.g)<<16|u32(color.b)<<8|color.a;
    fogclr=(fogclr&0xff000000)|((rgba>>8)&0xffffff);
    SC_TEV_FOG_COLOR_SET_RID(fogclr, 0xF2);

    GDWriteBPCmd(fog0);
    GDWriteBPCmd(fog1);
    GDWriteBPCmd(fog2);
    GDWriteBPCmd(fog3);
    GDWriteBPCmd(fogclr);
    
}

