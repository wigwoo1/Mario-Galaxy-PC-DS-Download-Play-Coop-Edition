#include <revolution/gx/GXPixel.h>
#include <revolution/gd/GDBase.h>
#include <stdexcept>
extern "C" void GXSetDstAlpha(GXBool enabled,u8 alpha){
 if(unsigned(enabled)>1)throw std::invalid_argument("GX destination alpha enable");
 // Original PE_CMODE1 constant alpha bits0..7 and enable bit8.
 // A BP mask preserves other retained fields without duplicate native state.
 GDOverflowCheck(10);GDWriteBPCmd(0xfe0001ffu);GDWriteBPCmd(0x42000000u|u32(alpha)|(u32(enabled)<<8));
}
