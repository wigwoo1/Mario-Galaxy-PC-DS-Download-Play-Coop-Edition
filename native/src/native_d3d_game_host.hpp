#pragma once

#include "native_d3d_command_submitter.hpp"
#include "native_d3d_xfb_present.hpp"
#include <Native/CommandSubmission.hpp>
#include <Native/GameInput.hpp>
#include <Native/GpuResources.hpp>
#include <stdexcept>

// Owns the native window and graphics objects required by one game instance.
// Construct before the original game's first GX flush and keep alive until exit.
class NativeD3dGameHost {
    template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
    unsigned efbWidth;
    unsigned efbHeight;
    struct Bootstrap {
        HWND window = nullptr;
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;

        Bootstrap(unsigned width, unsigned height, bool visible, bool warp) {
            if (!width || !height || width > 1024 || height > 1024)
                throw std::invalid_argument("Invalid native EFB dimensions");
            const auto instance = GetModuleHandleW(nullptr);
            WNDCLASSEXW wc{};
            wc.cbSize = sizeof(wc); wc.style = CS_OWNDC; wc.lpfnWndProc = DefWindowProcW;
            wc.hInstance = instance; wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
            wc.lpszClassName = L"GalaxyNativeGameWindow";
            if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
                throw std::runtime_error("Cannot register native game window class");
            RECT bounds{0, 0, 960, 720};
            if (!AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE))
                throw std::runtime_error("Cannot size native game window");
            window = CreateWindowExW(0, wc.lpszClassName, L"Super Mario Galaxy (Native)",
                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                bounds.right - bounds.left, bounds.bottom - bounds.top,
                nullptr, nullptr, instance, nullptr);
            if (!window) throw std::runtime_error("Cannot create native game window");

            D3D_FEATURE_LEVEL requested[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
                                              D3D_FEATURE_LEVEL_10_0};
            const auto driver = warp ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;
            HRESULT result = D3D11CreateDevice(nullptr, driver, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                requested, ARRAYSIZE(requested), D3D11_SDK_VERSION, &device, nullptr, &context);
            if (FAILED(result) && !warp)
                result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
                    D3D11_CREATE_DEVICE_BGRA_SUPPORT, requested, ARRAYSIZE(requested),
                    D3D11_SDK_VERSION, &device, nullptr, &context);
            if (FAILED(result)) {
                DestroyWindow(window); window = nullptr;
                throw std::runtime_error("Cannot create native D3D11 device");
            }
            if (visible) { ShowWindow(window, SW_SHOW); UpdateWindow(window); }
        }
        ~Bootstrap() {
            context.Reset(); device.Reset();
            if (window && IsWindow(window)) DestroyWindow(window);
            if (window) UnregisterClassW(L"GalaxyNativeGameWindow", GetModuleHandleW(nullptr));
        }
        Bootstrap(const Bootstrap&) = delete;
        Bootstrap& operator=(const Bootstrap&) = delete;
    };

    Bootstrap bootstrap;
    Native::D3dEfb efbTarget;
    Native::GpuResources& storage;
    NativeD3dXfbPresenter framePresenter;
    NativeD3dCommandSubmitter submitter;
    Native::CommandSubmissionBinding commandBinding;

    static NativeD3dGameHost*& activeHost() noexcept {
        static NativeD3dGameHost* value = nullptr;
        return value;
    }

public:
    NativeD3dGameHost(unsigned width, unsigned height, bool visible = true, bool warp = false)
        : efbWidth(width), efbHeight(height), bootstrap(width, height, visible, warp),
          efbTarget(*bootstrap.context.Get(), width, height),
          storage(Native::gpuResources()),
          framePresenter(*bootstrap.device.Get(), *bootstrap.context.Get(), bootstrap.window),
          submitter(*bootstrap.context.Get(), efbTarget, storage, {},
              [this](const Native::GxCommandState::CopyOperation& operation,
                     const std::uint8_t* bytes, std::size_t) {
                  framePresenter.presentYuyv(bytes, operation.width(), operation.height(), operation.strideBytes());
              }),
          commandBinding(submitter) {
        const float black[4] = {0.f, 0.f, 0.f, 1.f};
        efbTarget.clear(black, 1.f);
    }

    static NativeD3dGameHost& initialize(unsigned width, unsigned height,
                                         bool visible = true, bool warp = false) {
        auto*& host = activeHost();
        if (host) {
            if (host->efbWidth != width || host->efbHeight != height)
                throw std::logic_error("Native game host was initialized with a different EFB mode");
            return *host;
        }
        host = new NativeD3dGameHost(width, height, visible, warp);
        nativeBindGameInputWindow(host->window());
        return *host;
    }

    static NativeD3dGameHost& instance() {
        auto* host = activeHost();
        if (!host) throw std::logic_error("Native game host is not initialized");
        return *host;
    }

    HWND window() const noexcept { return bootstrap.window; }
    Native::GpuResources& resources() noexcept { return storage; }
    Native::D3dEfb& efb() noexcept { return efbTarget; }
    NativeD3dXfbPresenter& presenter() noexcept { return framePresenter; }

    bool pumpMessages() {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) return false;
            TranslateMessage(&message); DispatchMessageW(&message);
        }
        return IsWindow(bootstrap.window) != FALSE;
    }
};
