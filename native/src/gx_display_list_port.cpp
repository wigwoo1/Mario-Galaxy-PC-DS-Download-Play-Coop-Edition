#include <revolution/gd/GDBase.h>
#include <revolution/gx/GXDispList.h>
#include <memory>
#include <vector>

namespace {
struct Capture {
    GDLObj commands{};
    GDLObj* previous = nullptr;
};
thread_local std::vector<std::unique_ptr<Capture>> captures;
}

extern "C" void GXBeginDisplayList(void* storage, u32 size) {
    if (!__GDCurrentDL) return;
    auto capture = std::make_unique<Capture>();
    capture->previous = __GDCurrentDL;
    GDInitGDLObj(&capture->commands, storage, size);
    auto* selected = &capture->commands;
    captures.push_back(std::move(capture));
    __GDCurrentDL = selected;
}

extern "C" u32 GXEndDisplayList(void) {
    if (captures.empty()) return 0;
    const u32 bytes = GDGetCurrOffset();
    __GDCurrentDL = captures.back()->previous;
    captures.pop_back();
    return bytes;
}
