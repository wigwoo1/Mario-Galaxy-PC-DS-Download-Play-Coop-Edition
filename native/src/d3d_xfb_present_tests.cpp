#include "native_d3d_xfb_present.hpp"
#include <cassert>
#include <cstdio>

int main() {
    std::array<std::uint8_t, 32> xfb{};
    xfb.fill(0xa5); // Padding must not be interpreted as pixels.
    xfb[0] = 82; xfb[1] = 90; xfb[2] = 82; xfb[3] = 240;
    const auto decoded = NativeD3dXfbPresenter::decodeYuyv(xfb.data(), 2, 1, 32);
    assert(decoded.size() == 8);
    assert(decoded[0] > 250 && decoded[1] < 8 && decoded[2] < 8 && decoded[3] == 255);
    assert(decoded[4] > 250 && decoded[5] < 8 && decoded[6] < 8 && decoded[7] == 255);
    const std::array<std::uint8_t, 4> black{16, 128, 16, 128};
    const auto blackRgba = NativeD3dXfbPresenter::decodeYuyv(black.data(), 2, 1, 4);
    assert((blackRgba == std::vector<std::uint8_t>{0, 0, 0, 255, 0, 0, 0, 255}));
    bool rejected = false;
    try { (void)NativeD3dXfbPresenter::decodeYuyv(xfb.data(), 3, 1, 32); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"GalaxyNativeXfbPresentTest";
    assert(RegisterClassW(&wc));
    HWND window = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPED,
                                  0, 0, 80, 40, nullptr, nullptr, wc.hInstance, nullptr);
    assert(window);
    RECT client{}; assert(GetClientRect(window, &client));
    assert(client.right > client.left && client.bottom > client.top);
    ID3D11Device* rawDevice = nullptr;
    ID3D11DeviceContext* rawContext = nullptr;
    const auto created = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
                                            D3D11_SDK_VERSION, &rawDevice, nullptr, &rawContext);
    assert(SUCCEEDED(created));
    Microsoft::WRL::ComPtr<ID3D11Device> device; device.Attach(rawDevice);
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context; context.Attach(rawContext);
    {
        NativeD3dXfbPresenter presenter(*device.Get(), *context.Get(), window);
        presenter.renderYuyv(xfb.data(), 2, 1, 32);
        const auto frame = presenter.readbackBackbuffer();
        const unsigned clientWidth = static_cast<unsigned>(client.right - client.left);
        const unsigned clientHeight = static_cast<unsigned>(client.bottom - client.top);
        assert(frame.size() == std::size_t(clientWidth) * clientHeight * 4);
        const auto center = (std::size_t(clientHeight / 2) * clientWidth + clientWidth / 2) * 4;
        assert(frame[center] > 240 && frame[center + 1] < 20 && frame[center + 2] < 20);
        // GX's depth-only clear leaves color writes disabled on the same
        // immediate context. Presentation must establish its own output state.
        D3D11_BLEND_DESC depthOnlyBlend{};
        Microsoft::WRL::ComPtr<ID3D11BlendState> noColorWrites;
        assert(SUCCEEDED(device->CreateBlendState(&depthOnlyBlend, &noColorWrites)));
        context->OMSetBlendState(noColorWrites.Get(), nullptr, 0xffffffffu);
        presenter.renderYuyv(xfb.data(), 2, 1, 32);
        const auto afterDepthClear = presenter.readbackBackbuffer();
        assert(afterDepthClear[center] > 240 && afterDepthClear[center + 1] < 20 && afterDepthClear[center + 2] < 20);
        presenter.presentFrame(); // Present the exact frame that was read back.
        presenter.presentYuyv(xfb.data(), 2, 1, 32);
    }
    context.Reset(); device.Reset();
    DestroyWindow(window); UnregisterClassW(wc.lpszClassName, wc.hInstance);
    std::puts("PASS YUYV XFB -> BT.601 RGB -> D3D11 backbuffer -> DXGI Present; depth-only color-mask isolation (WARP)");
}
