#include <revolution/gx/GXTexture.h>
#include <revolution/gd/GDBase.h>
#include <array>
#include <stdexcept>

namespace {
std::array<u32, 8> scaleS{};
std::array<u32, 8> scaleT{};
u32 manualScaleMask{};
struct InitScaleRegisters {
    InitScaleRegisters() {
        for (u32 coord = 0; coord < 8; ++coord) {
            scaleS[coord] = (0x30u + coord * 2u) << 24;
            scaleT[coord] = (0x31u + coord * 2u) << 24;
        }
    }
} initScaleRegisters;
}

extern "C" void GXSetTexCoordScaleManually(GXTexCoordID coord, GXBool enable, u16 ss, u16 ts) {
    const u32 index = static_cast<u32>(coord);
    if (index >= 8 || static_cast<u32>(enable) > 1)
        throw std::invalid_argument("GX texture coordinate scale selector");

    const u32 bit = 1u << index;
    if (!enable) {
        manualScaleMask &= ~bit;
        return;
    }

    const u32 nextS = (scaleS[index] & 0xffff0000u) | static_cast<u16>(ss - 1u);
    const u32 nextT = (scaleT[index] & 0xffff0000u) | static_cast<u16>(ts - 1u);
    GDOverflowCheck(10);
    GDWriteBPCmd(nextS);
    GDWriteBPCmd(nextT);
    scaleS[index] = nextS;
    scaleT[index] = nextT;
    manualScaleMask |= bit;
}

extern "C" void GXTexModeSync(void) {
    GDOverflowCheck(5);
    GDWriteBPCmd(0x63000000u);
}
