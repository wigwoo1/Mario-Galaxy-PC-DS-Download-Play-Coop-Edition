#include <revolution/gx/GXGeometry.h>
#include <revolution/gd/GDBase.h>
// Original GXAttr.c emits the one-byte invalidate command to the FIFO.
extern "C" void GXInvalidateVtxCache(){GDOverflowCheck(1);GDWrite_u8(0x48);}
