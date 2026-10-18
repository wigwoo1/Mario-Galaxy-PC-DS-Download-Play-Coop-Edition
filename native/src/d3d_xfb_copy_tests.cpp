#include "native_d3d_xfb_copy.hpp"
#include "native_d3d_command_submitter.hpp"
#include <Native/CommandSubmission.hpp>
#include <Native/DisplayCopyBinding.hpp>
#include <revolution/gd/GDBase.h>
#include <revolution/gx.h>
#include <cassert>
#include <cstdio>

int main() {
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    assert(SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
                                      D3D11_SDK_VERSION, &device, nullptr, &context)));

    Native::D3dEfb efb(*context.Get(), 16, 4);
    const float red[] = {1, 0, 0, 1};
    efb.clear(red, 1.f);
    Native::GpuResources resources;
    std::array<std::uint8_t, 128> xfb;
    xfb.fill(0xa5);
    const auto address = resources.addWritable(xfb.data(), xfb.size());

    Native::GxCommandState::CopyOperation copy{};
    copy.registers[0x4a] = (3u << 10) | 3u;
    copy.registers[0x4b] = address >> 5;
    copy.registers[0x4d] = 1u; // 32-byte stride
    copy.registers[0x52] = 1u << 14; // display/XFB copy
    // Native progressive filter [0, 0, 21, 22, 21, 0, 0].
    copy.registers[0x53] = (21u << 12) | (22u << 18);
    copy.registers[0x54] = 21u;

    NativeD3dXfbCopy convert(efb, resources, *context.Get());
    convert(copy);
    assert((std::array<std::uint8_t, 4>{xfb[0], xfb[1], xfb[2], xfb[3]} ==
            std::array<std::uint8_t, 4>{82, 90, 82, 240}));
    assert((std::array<std::uint8_t, 4>{xfb[4], xfb[5], xfb[6], xfb[7]} ==
            std::array<std::uint8_t, 4>{82, 90, 82, 240}));
    assert(xfb[8] == 0xa5 && xfb[31] == 0xa5 && xfb[32] == 82);

    copy.registers[0x52] |= 1u << 10;
    bool scalingRejected = false;
    try { convert(copy); } catch (const std::invalid_argument&) { scalingRejected = true; }
    assert(scalingRejected);

    // Exercise the production command submitter's default display-copy route.
    xfb.fill(0xa5);
    Native::GxCommandState mirror;
    alignas(32) std::array<std::uint8_t, 1024> commands{};
    GDLObj list{};
    GDInitGDLObj(&list, commands.data(), commands.size());
    GDSetCurrent(&list);
    {
        Native::DisplayCopyBinding displayBinding(mirror, resources);
        bool deliveredToPresenter = false;
        NativeD3dCommandSubmitter submitter(*context.Get(), efb, resources, {},
            [&](const Native::GxCommandState::CopyOperation& operation,
                const std::uint8_t* bytes, std::size_t length) {
                assert(operation.display() && operation.width() == 16 && operation.height() == 4);
                assert(operation.strideBytes() == 32 && operation.destinationAddress() == address);
                assert(length == xfb.size() && bytes[0] == 82 && bytes[1] == 90 && bytes[2] == 82 && bytes[3] == 240);
                assert(bytes[124] == 82 && bytes[126] == 82 && bytes[127] == 240);
                deliveredToPresenter = true;
            });
        Native::CommandSubmissionBinding submissionBinding(submitter);
        GDWriteBPCmd(0x40000012);
        GDWriteBPCmd(0x41000003);
        GDWriteBPCmd(0x43000040);
        GXDrawDone(); // Frame-loop-style flush boundary before GXCopyDisp.
        assert(mirror.bp(0x40) == 0x12 && mirror.bp(0x41) == 0x03 && mirror.bp(0x43) == 0x40);
        GXSetDispCopySrc(0, 0, 16, 4);
        GXSetDispCopyDst(16, 4);
        GXSetDispCopyYScale(1.f);
        GXSetDispCopyGamma(GX_GM_1_0);
        GXSetDispCopyFrame2Field(GX_COPY_PROGRESSIVE);
        GXSetCopyClamp(static_cast<GXFBClamp>(GX_CLAMP_TOP | GX_CLAMP_BOTTOM));
        GDWriteBPCmd(0x4f00ff00);
        GDWriteBPCmd(0x5000ff00);
        GDWriteBPCmd(0x51ffffff);
        GXCopyDisp(xfb.data(), GX_TRUE);
        GXDrawDone();
        assert(deliveredToPresenter);
    }
    assert(GDGetCurrOffset() == 0);
    assert((std::array<std::uint8_t, 4>{xfb[0], xfb[1], xfb[2], xfb[3]} ==
            std::array<std::uint8_t, 4>{82, 90, 82, 240}));
    const auto clearedEfb = efb.readColor();
    assert(clearedEfb[0] == 0 && clearedEfb[1] == 255 && clearedEfb[2] == 0 && clearedEfb[3] == 255);
    GDSetCurrent(nullptr);
    resources.remove(xfb.data());
    std::puts("PASS D3D EFB -> progressive Wii XFB -> display-frame callback, stride preservation, scaling rejection and native GX submission integration");
}
