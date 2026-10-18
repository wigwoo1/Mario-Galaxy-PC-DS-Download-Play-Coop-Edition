#include "native_d3d_game_host.hpp"
#include <Native/DisplayCopyBinding.hpp>
#include <Native/TextureCopyBinding.hpp>
#include <Native/GxCommandState.hpp>
#include <Native/GameInput.hpp>
#include <revolution/gd/GDBase.h>
#include <revolution/gx.h>
#include <revolution/wpad.h>
#include <array>
#include <cassert>
#include <cstdio>

int main() {
    NativeD3dGameHost& host = NativeD3dGameHost::initialize(16, 4, false, true);
    assert(&NativeD3dGameHost::instance() == &host);
    nativePumpGameInputFrame();
    u32 inputDeviceType = WPAD_DEV_NOT_FOUND;
    assert(WPADProbe(WPAD_CHAN0, &inputDeviceType) == WPAD_ERR_NONE);
    assert(inputDeviceType == WPAD_DEV_FREESTYLE);
    const float red[4] = {1.f, 0.f, 0.f, 1.f};
    host.efb().clear(red, 1.f);

    alignas(32) std::array<std::uint8_t, 128> xfb{};
    xfb.fill(0xa5);
    const auto address = host.resources().addWritable(xfb.data(), xfb.size());
    alignas(32) std::array<std::uint8_t, 1024> commands{};
    GDLObj list{};
    GDInitGDLObj(&list, commands.data(), commands.size());
    GDSetCurrent(&list);
    Native::GxCommandState mirror;
    {
        Native::DisplayCopyBinding displayBinding(mirror, host.resources());
        GXSetDispCopySrc(0, 0, 16, 4);
        GXSetDispCopyDst(16, 4);
        GXSetDispCopyYScale(1.f);
        GXSetDispCopyGamma(GX_GM_1_0);
        GXSetDispCopyFrame2Field(GX_COPY_PROGRESSIVE);
        GXSetCopyClamp(static_cast<GXFBClamp>(GX_CLAMP_TOP | GX_CLAMP_BOTTOM));
        GXCopyDisp(xfb.data(), GX_FALSE);
        GXDrawDone(); // Executes through the host's persistent native submitter.
        alignas(32) std::array<std::uint8_t,128> captured{};
        host.resources().addWritable(captured.data(),captured.size());
        {
            Native::TextureCopyBinding textureBinding(mirror,host.resources());
            // State changes remain in the unflushed command writer until capture.
            GDWriteBPCmd(0x40000002);GDWriteBPCmd(0x41000003);GDWriteBPCmd(0x43000040);
            GXSetTexCopySrc(0,0,16,4);GXSetTexCopyDst(16,4,GX_TF_RGB565,GX_FALSE);
            GXCopyTex(captured.data(),GX_TRUE);
            GXPixModeSync();
            assert(mirror.bp(0x40)==2&&mirror.bp(0x41)==3&&mirror.bp(0x43)==64);
            GXDrawDone();
            assert(captured[0]==0xf8&&captured[1]==0&&captured[126]==0xf8&&captured[127]==0);
            alignas(32) std::array<std::uint8_t,128> alpha{};alpha.fill(0xa5);
            host.resources().addWritable(alpha.data(),alpha.size());
            const float translucent[]={1.f,0.f,0.f,128.f/255};host.efb().clear(translucent,1.f);
            GDWriteBPCmd(0x43000001); // RGBA6 EFB: six-bit alpha is expanded on capture.
            GDWriteBPCmd(0x53000001);GDWriteBPCmd(0x54000002); // RGB-only copy filtering cannot change captured alpha.
            GXSetTexCopySrc(2,1,9,2);GXSetTexCopyDst(9,2,GX_CTF_A8,GX_FALSE);
            GXCopyTex(alpha.data(),GX_TRUE);GXPixModeSync();GXDrawDone();
            for(unsigned y=0;y<2;++y)for(unsigned x=0;x<9;++x)assert(alpha[(x/8)*32+y*8+x%8]==130);
            assert(alpha[16]==0xa5&&alpha[34]==0xa5&&alpha[127]==0xa5);
            auto after=host.efb().readColor();assert(after[(1*16+2)*4+3]==0&&after[(0*16+0)*4+3]==128);
            assert(std::memcmp(alpha.data(),host.resources().resolve(host.resources().address(alpha.data()),alpha.size()),alpha.size())==0);
            host.resources().remove(alpha.data());
            std::puts("PASS original GX A8 capture on native host: RGBA6 alpha, tile padding, snapshot, copy-before-clear and preserved outside rectangle");
        }
        host.resources().remove(captured.data());
    }
    assert((std::array<std::uint8_t, 4>{xfb[0], xfb[1], xfb[2], xfb[3]} ==
            std::array<std::uint8_t, 4>{82, 90, 82, 240}));
    assert(xfb[124] == 82 && xfb[126] == 82 && xfb[127] == 240);
    host.resources().remove(xfb.data());
    GDSetCurrent(nullptr);
    std::puts("PASS game host window/device/EFB/GX submitter/XFB registry/swap-chain integration on WARP");
}
