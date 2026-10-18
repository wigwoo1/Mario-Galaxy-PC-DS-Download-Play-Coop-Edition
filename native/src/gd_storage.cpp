#include <revolution/gd/GDBase.h>
#include <cstdint>
#include <stdexcept>

extern "C" {
thread_local GDLObj* __GDCurrentDL = nullptr;

void GDInitGDLObj(GDLObj* dl, void* storage, u32 length) {
    if (!dl || !storage || !length ||
        reinterpret_cast<std::uintptr_t>(storage) > UINTPTR_MAX - length)
        throw std::invalid_argument("Invalid display-list storage");
    auto* bytes = static_cast<u8*>(storage);
    *dl = {bytes, length, bytes, bytes + length};
}

void GDOverflowed() {
    throw std::length_error("Display-list storage exhausted");
}

void GDPadCurr32() {
    if (!__GDCurrentDL) throw std::logic_error("No current display list");
    const auto offset = GDGetCurrOffset();
    const u32 padding = (32 - (offset & 31)) & 31;
    GDOverflowCheck(padding);
    for (u32 i = 0; i < padding; ++i) *__GDCurrentDL->ptr++ = 0;
}

void GDFlushCurrToMem() {
    // Host display-list storage is ordinary coherent CPU memory.
    if (!__GDCurrentDL) throw std::logic_error("No current display list");
    GDOverflowCheck(0);
}
}
