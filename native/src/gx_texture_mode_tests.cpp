#include <revolution/gx/GXTexture.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
#include <stdexcept>

extern "C" { GDLObj* __GDCurrentDL = nullptr; }
extern "C" void GDOverflowed() { std::abort(); }

int main() {
    u8 bytes[32]{};
    GDLObj list{bytes, sizeof(bytes), bytes, bytes + sizeof(bytes)};
    __GDCurrentDL = &list;

    GXSetTexCoordScaleManually(GX_TEXCOORD2, GX_TRUE, 256, 128);
    assert(list.ptr - bytes == 10);
    const u8 expected[] = {0x61,0x34,0x00,0x00,0xff, 0x61,0x35,0x00,0x00,0x7f};
    for (unsigned i=0; i<sizeof(expected); ++i) assert(bytes[i] == expected[i]);

    const auto beforeDisable = list.ptr;
    GXSetTexCoordScaleManually(GX_TEXCOORD2, GX_FALSE, 0, 0);
    assert(list.ptr == beforeDisable);
    GXTexModeSync();
    assert(list.ptr - bytes == 15);
    const u8 sync[] = {0x61,0x63,0x00,0x00,0x00};
    for (unsigned i=0; i<sizeof(sync); ++i) assert(bytes[10+i] == sync[i]);

    bool rejected=false;
    try { GXSetTexCoordScaleManually(static_cast<GXTexCoordID>(8), GX_TRUE, 1, 1); }
    catch (const std::invalid_argument&) { rejected=true; }
    assert(rejected && list.ptr-bytes==15);
    std::puts("PASS GX manual texcoord scales and texture-mode sync: BP register IDs, dimensions, disable and validation");
}
