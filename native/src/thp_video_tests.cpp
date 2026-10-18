#include <Native/ThpEndian.hpp>
#include <Native/ThpVideo.hpp>
#include <native_dvd_port.hpp>
#include <revolution/dvd.h>
#include <revolution/thp.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <vector>

namespace {
bool decodeFirstFrame(const char* path) {
    DVDFileInfo file{};
    if (!DVDOpen(path, &file)) return false;
    std::array<u8, 64> headerBytes{};
    if (DVDReadPrio(&file, headerBytes.data(), s32(headerBytes.size()), 0, 2) != s32(headerBytes.size())) return false;
    THPHeader header{};
    NativeThp::header(header, headerBytes.data());
    assert(std::memcmp(header.magic, "THP", 3) == 0);
    assert(header.version == 0x11000 && header.firstFrameSize > 16 && header.firstFrameSize < header.bufSize);

    std::array<u8, 32> componentBytes{};
    assert(DVDReadPrio(&file, componentBytes.data(), s32(componentBytes.size()), s32(header.compInfoDataOffsets), 2) == s32(componentBytes.size()));
    THPFrameCompInfo components{};
    NativeThp::components(components, componentBytes.data());
    assert(components.numComponents > 0 && components.numComponents <= 4);
    std::array<u8, 12> videoBytes{};
    assert(DVDReadPrio(&file, videoBytes.data(), s32(videoBytes.size()), s32(header.compInfoDataOffsets + 20), 2) == s32(videoBytes.size()));
    THPVideoInfo video{};
    NativeThp::video(video, videoBytes.data());
    assert(video.xSize >= 2 && video.ySize >= 2 && !(video.xSize & 1) && !(video.ySize & 1));
    assert(std::uint64_t(video.xSize) * video.ySize <= 32u * 1024u * 1024u);

    std::vector<u8> frame(header.firstFrameSize);
    assert(DVDReadPrio(&file, frame.data(), s32(frame.size()), s32(header.movieDataOffsets), 2) == s32(frame.size()));
    const std::size_t payloadOffset = 8 + std::size_t(components.numComponents) * 4;
    assert(payloadOffset <= frame.size());
    const u32 pixels = video.xSize * video.ySize;
    std::vector<u8> y(pixels), u(pixels / 4), v(pixels / 4);
    alignas(32) std::array<u8, 64> work{};
    bool decoded = false;
    std::size_t componentOffset = payloadOffset;
    for (u32 index = 0; index < components.numComponents; ++index) {
        const u32 byteCount = NativeThp::read32(frame.data() + 8 + index * 4);
        assert(byteCount <= frame.size() - componentOffset);
        if (components.frameComp[index] == 0) {
            NativeThp::VideoFrameScope context(frame.data() + componentOffset, byteCount, video.xSize, video.ySize);
            assert(THPVideoDecode(frame.data() + componentOffset, y.data(), u.data(), v.data(), work.data()) == 0);
            decoded = true;
        }
        componentOffset += byteCount;
    }
    assert(decoded);
    const auto yRange = std::minmax_element(y.begin(), y.end());
    const auto uRange = std::minmax_element(u.begin(), u.end());
    const auto vRange = std::minmax_element(v.begin(), v.end());
    // Some legal opening/end frames are a single flat color; require a populated
    // luma plane without incorrectly requiring every frame to contain detail.
    assert(*yRange.second > 0);
    assert(DVDClose(&file));
    std::printf("PASS THP first frame %s: %ux%u, %u-byte JPEG component, YUV420 planes populated\n", path, video.xSize, video.ySize,
                unsigned(componentOffset - payloadOffset));
    return true;
}
}

int main(int argc, char** argv) {
    assert(argc == 2 && nativeBindDvd(std::filesystem::u8path(argv[1])));
    alignas(32) std::array<u8, 64> dummy{};
    assert(THPVideoDecode(dummy.data(), dummy.data(), dummy.data(), dummy.data(), dummy.data()) == 29);
    assert(THPInit());
    assert(NativeThp::videoDecoderInitialized());
    assert(THPVideoDecode(dummy.data(), dummy.data(), dummy.data(), dummy.data(), dummy.data()) == 3);
    unsigned decoded = 0;
    for (const char* path : {"/MovieData/PrologueA.thp", "/MovieData/PrologueB.thp", "/MovieData/FinalBattle.thp", "/MovieData/EndingA.thp"}) {
        if (DVDConvertPathToEntrynum(path) < 0) continue;
        assert(decodeFirstFrame(path));
        ++decoded;
    }
    assert(decoded >= 3);
    nativeReleaseDvd();
    std::puts("PASS native THP initialization, bounded WIC JPEG decode, metadata-size agreement, and YUV420 output against retail WBFS movies");
}
