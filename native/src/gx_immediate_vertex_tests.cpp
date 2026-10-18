#include <Native/GxImmediateVertices.hpp>
#include <revolution/gd/GDBase.h>
#include <array>
#include <cassert>
#include <cstdio>
#include <stdexcept>

GDLObj* __GDCurrentDL = nullptr;
void GDOverflowed() { throw std::overflow_error("display list overflow"); }

int main() {
    bool missing = false;
    try { Native::writePositionTexCoord(0, 0, 0, 0, 0); }
    catch (const std::logic_error&) { missing = true; }
    assert(missing);

    std::array<u8, 20> bytes{};
    GDLObj list{bytes.data(), static_cast<u32>(bytes.size()), bytes.data(), bytes.data() + bytes.size()};
    GDSetCurrent(&list);
    Native::writePositionTexCoord(1.0f, -2.0f, 0.5f, 0.25f, 1.0f);
    const std::array<u8, 20> expected{{
        0x3f,0x80,0x00,0x00, 0xc0,0x00,0x00,0x00, 0x3f,0x00,0x00,0x00,
        0x3e,0x80,0x00,0x00, 0x3f,0x80,0x00,0x00
    }};
    assert(list.ptr == list.start + expected.size());
    assert(bytes == expected);

    std::array<u8, 19> shortBytes{};
    GDLObj shortList{shortBytes.data(), static_cast<u32>(shortBytes.size()), shortBytes.data(), shortBytes.data() + shortBytes.size()};
    GDSetCurrent(&shortList);
    bool overflow = false;
    try { Native::writePositionTexCoord(1, 2, 3, 4, 5); }
    catch (const std::overflow_error&) { overflow = true; }
    assert(overflow && shortList.ptr == shortList.start);
    GDSetCurrent(nullptr);
    std::puts("PASS native GX immediate vertex: exact big-endian XYZ/ST stream, missing-list and overflow checks");
}
