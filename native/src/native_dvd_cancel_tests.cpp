#include "native_dvd_port.hpp"
#include <revolution/dvd.h>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <vector>

namespace {
std::atomic<s32> callbackResult{-999};
std::atomic<unsigned> callbackCount{0};
void readComplete(s32 result, DVDFileInfo*) {
    callbackResult.store(result);
    callbackCount.fetch_add(1);
}
}

int main(int argc, char** argv) {
    assert(argc == 2);
    assert(nativeBindDvd(std::filesystem::u8path(argv[1])));
    const s32 entry = DVDConvertPathToEntrynum("/ObjectData/Mario.arc");
    assert(entry > 0);

    DVDFileInfo file{};
    assert(DVDFastOpen(entry, &file));
    assert(file.length > 0 && file.length < 64 * 1024 * 1024);
    std::vector<u8> destination(file.length);
    std::vector<u8> expected(file.length);
    assert(DVDReadPrio(&file, expected.data(), s32(expected.size()), 0, 2) == s32(expected.size()));
    assert(file.cb.addr == expected.data());
    std::fill(destination.begin(), destination.end(), 0);
    callbackResult.store(-999);
    callbackCount.store(0);
    assert(DVDReadAsyncPrio(&file, destination.data(), s32(destination.size()), 0, readComplete, 2));
    assert(DVDClose(&file));
    assert(callbackCount.load() == 1);
    assert(callbackResult.load() == s32(expected.size()));
    assert(destination == expected);
    assert(file.cb.addr == destination.data());
    assert(DVDFastOpen(entry, &file));
    unsigned cancelled = 0;
    for (unsigned attempt = 0; attempt < 16 && !cancelled; ++attempt) {
        std::fill(destination.begin(), destination.end(), 0xA5);
        callbackResult.store(-999);
        callbackCount.store(0);
        assert(DVDReadAsyncPrio(&file, destination.data(), s32(destination.size()), 0, readComplete, 2));
        assert(DVDCancel(&file.cb) == 0);
        assert(callbackCount.load() == 1);
        const s32 result = callbackResult.load();
        if (result == -3) {
            ++cancelled;
            assert(file.cb.state == DVD_STATE_CANCELED);
            assert(file.cb.transferredSize == 0);
            assert(std::all_of(destination.begin(), destination.end(), [](u8 value) { return value == 0xA5; }));
        } else {
            assert(result == s32(destination.size()));
            assert(file.cb.state == 0);
            assert(DVDGetCommandBlockStatus(&file.cb) == 0);
        }
    }
    assert(cancelled > 0 && "fixture did not overlap cancellation with the active read");
    assert(DVDClose(&file));
    nativeReleaseDvd();
    std::puts("PASS native DVD cancellation: active read reports -3 once, preserves caller buffer, reaches canceled state and synchronizes close");
}
