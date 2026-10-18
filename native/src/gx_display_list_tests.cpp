#include <revolution/gd/GDBase.h>
#include <revolution/gx/GXDispList.h>
#include <cassert>
#include <cstdio>
#include <stdexcept>

int main() {
    u8 outer[64]{}, inner[32]{};
    GDLObj outerList{};
    GDInitGDLObj(&outerList, outer, sizeof(outer));
    GDSetCurrent(&outerList);
    GDWrite_u8(0x31);

    GXBeginDisplayList(inner, sizeof(inner));
    assert(__GDCurrentDL != &outerList);
    GDWriteCPCmd(0x09, 0x12345678);
    const u32 innerSize = GXEndDisplayList();
    assert(innerSize == 6);
    assert(__GDCurrentDL == &outerList);
    assert(inner[0] == 0x08 && inner[1] == 0x09);
    assert(inner[2] == 0x12 && inner[3] == 0x34 && inner[4] == 0x56 && inner[5] == 0x78);
    assert(GDGetCurrOffset() == 1 && outer[0] == 0x31);

    u8 nested[16]{};
    GXBeginDisplayList(inner, sizeof(inner));
    GDWrite_u8(0xA1);
    GXBeginDisplayList(nested, sizeof(nested));
    GDWrite_u16(0xBEEF);
    assert(GXEndDisplayList() == 2);
    assert(GDGetCurrOffset() == 1);
    assert(GXEndDisplayList() == 1);
    assert(__GDCurrentDL == &outerList && GDGetCurrOffset() == 1);

    u8 tooSmall[2]{};
    GXBeginDisplayList(tooSmall, sizeof(tooSmall));
    bool overflowed = false;
    try { GDWrite_u32(0xDEADBEEF); } catch (const std::length_error&) { overflowed = true; }
    assert(overflowed);
    (void)GXEndDisplayList();
    assert(__GDCurrentDL == &outerList);
    GDSetCurrent(nullptr);
    GXBeginDisplayList(inner, sizeof(inner));
    assert(__GDCurrentDL == nullptr && GXEndDisplayList() == 0);
    std::puts("PASS native GX display-list capture, nesting, bounds and context restore");
    return 0;
}
