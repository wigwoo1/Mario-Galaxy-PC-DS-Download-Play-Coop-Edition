#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

// Converts the progressive YUYV XFB produced by NativeD3dXfbCopy to a
// letterboxed D3D11 swap-chain frame. Call on the thread that owns the
// immediate context; the game host still owns the HWND and message pump.
template<class T> using NativeD3dComPtr = Microsoft::WRL::ComPtr<T>;
class NativeD3dXfbPresenter {
    NativeD3dComPtr<ID3D11Device> device;
    NativeD3dComPtr<ID3D11DeviceContext> context;
    NativeD3dComPtr<IDXGISwapChain> swap;
    NativeD3dComPtr<ID3D11Texture2D> frame;
    NativeD3dComPtr<ID3D11ShaderResourceView> frameView;
    NativeD3dComPtr<ID3D11VertexShader> vertexShader;
    NativeD3dComPtr<ID3D11PixelShader> pixelShader;
    NativeD3dComPtr<ID3D11SamplerState> sampler;
    NativeD3dComPtr<ID3D11RasterizerState> rasterizer;
    NativeD3dComPtr<ID3D11DepthStencilState> presentationDepth;
    NativeD3dComPtr<ID3D11RenderTargetView> windowTarget;
    HWND window = nullptr;
    unsigned frameWidth = 0, frameHeight = 0;
    unsigned targetWidth = 0, targetHeight = 0;

    static void check(HRESULT result, const char* operation) {
        if (FAILED(result))
            throw std::runtime_error(std::string(operation) + " failed (HRESULT " +
                                     std::to_string(static_cast<unsigned long>(result)) + ")");
    }

    void createFrame(unsigned width, unsigned height) {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width; desc.Height = height; desc.MipLevels = 1; desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        NativeD3dComPtr<ID3D11Texture2D> next;
        check(device->CreateTexture2D(&desc, nullptr, &next), "Create XFB frame texture");
        NativeD3dComPtr<ID3D11ShaderResourceView> view;
        check(device->CreateShaderResourceView(next.Get(), nullptr, &view), "Create XFB frame view");
        frame = std::move(next); frameView = std::move(view);
        frameWidth = width; frameHeight = height;
    }

    void resizeTarget(unsigned width, unsigned height) {
        if (!width || !height) return;
        if (width == targetWidth && height == targetHeight) return;
        context->OMSetRenderTargets(0, nullptr, nullptr);
        windowTarget.Reset();
        check(swap->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0), "Resize game swap chain");
        NativeD3dComPtr<ID3D11Texture2D> backBuffer;
        check(swap->GetBuffer(0, __uuidof(ID3D11Texture2D),
                              reinterpret_cast<void**>(backBuffer.GetAddressOf())), "Get game back buffer");
        check(device->CreateRenderTargetView(backBuffer.Get(), nullptr, &windowTarget), "Create game render target");
        targetWidth = width; targetHeight = height;
    }

    void drawFrame() {
        RECT client{};
        if (!GetClientRect(window, &client)) throw std::runtime_error("Cannot query game window size");
        const unsigned width = static_cast<unsigned>(std::max<LONG>(0, client.right - client.left));
        const unsigned height = static_cast<unsigned>(std::max<LONG>(0, client.bottom - client.top));
        if (!width || !height) return;
        resizeTarget(width, height);
        const float black[4] = {0.f, 0.f, 0.f, 1.f};
        context->ClearRenderTargetView(windowTarget.Get(), black);
        context->OMSetRenderTargets(1, windowTarget.GetAddressOf(), nullptr);
        // EFB rendering shares this context and can leave a depth-only clear's
        // zero color-write mask active. The window copy always writes RGBA.
        context->OMSetBlendState(nullptr, nullptr, 0xffffffffu);
        context->OMSetDepthStencilState(presentationDepth.Get(), 0);
        const float scale = std::min(float(width) / frameWidth, float(height) / frameHeight);
        const D3D11_VIEWPORT viewport{
            (width - frameWidth * scale) * 0.5f,
            (height - frameHeight * scale) * 0.5f,
            frameWidth * scale, frameHeight * scale, 0.f, 1.f};
        context->RSSetViewports(1, &viewport);
        context->RSSetState(rasterizer.Get());
        context->IASetInputLayout(nullptr);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(vertexShader.Get(), nullptr, 0);
        context->PSSetShader(pixelShader.Get(), nullptr, 0);
        auto view = frameView.Get(); auto sampling = sampler.Get();
        context->PSSetShaderResources(0, 1, &view);
        context->PSSetSamplers(0, 1, &sampling);
        context->Draw(3, 0);
        ID3D11ShaderResourceView* none = nullptr;
        context->PSSetShaderResources(0, 1, &none);
    }

public:
    NativeD3dXfbPresenter(ID3D11Device& gpu, ID3D11DeviceContext& immediate, HWND targetWindow)
        : window(targetWindow) {
        if (!window || !IsWindow(window)) throw std::invalid_argument("Invalid native game window");
        device = &gpu; context = &immediate;
        const char* shader = R"(
struct Varying { float4 position : SV_Position; float2 uv : TEXCOORD0; };
Varying vs(uint id : SV_VertexID) {
    Varying o; o.uv = float2((id << 1) & 2, id & 2);
    o.position = float4(o.uv.x * 2 - 1, 1 - o.uv.y * 2, 0, 1); return o;
}
Texture2D source : register(t0); SamplerState sampling : register(s0);
float4 ps(Varying i) : SV_Target { return float4(source.Sample(sampling, i.uv).rgb, 1); }
)";
        auto compile = [&](const char* entry, const char* profile, NativeD3dComPtr<ID3DBlob>& output) {
            NativeD3dComPtr<ID3DBlob> errors;
            HRESULT result = D3DCompile(shader, std::strlen(shader), "native-xfb-present", nullptr,
                                        nullptr, entry, profile, D3DCOMPILE_ENABLE_STRICTNESS, 0,
                                        &output, &errors);
            if (FAILED(result)) {
                const std::string detail = errors ? std::string(static_cast<const char*>(errors->GetBufferPointer()),
                                                                errors->GetBufferSize()) : "shader compile failed";
                throw std::runtime_error(detail);
            }
        };
        NativeD3dComPtr<ID3DBlob> vs, ps;
        compile("vs", "vs_4_0", vs); compile("ps", "ps_4_0", ps);
        check(device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &vertexShader), "Create XFB vertex shader");
        check(device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &pixelShader), "Create XFB pixel shader");

        NativeD3dComPtr<IDXGIDevice> dxgiDevice;
        check(device.As(&dxgiDevice), "Query DXGI device");
        NativeD3dComPtr<IDXGIAdapter> adapter;
        check(dxgiDevice->GetAdapter(&adapter), "Get DXGI adapter");
        NativeD3dComPtr<IDXGIFactory> factory;
        check(adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf())), "Get DXGI factory");
        DXGI_SWAP_CHAIN_DESC desc{};
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2;
        desc.OutputWindow = window;
        desc.Windowed = TRUE;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        check(factory->CreateSwapChain(device.Get(), &desc, &swap), "Create game swap chain");
        check(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER), "Configure game window association");

        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        check(device->CreateSamplerState(&samplerDesc, &sampler), "Create XFB sampler");
        D3D11_RASTERIZER_DESC rasterDesc{};
        rasterDesc.FillMode = D3D11_FILL_SOLID; rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.DepthClipEnable = TRUE;
        check(device->CreateRasterizerState(&rasterDesc, &rasterizer), "Create XFB rasterizer");
        D3D11_DEPTH_STENCIL_DESC depthDesc{};
        depthDesc.DepthEnable = FALSE;
        depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        check(device->CreateDepthStencilState(&depthDesc, &presentationDepth), "Create XFB depth state");
    }

    static std::vector<std::uint8_t> decodeYuyv(const std::uint8_t* source, unsigned width,
                                                unsigned height, unsigned stride) {
        if (!source || !width || !height || (width & 1) || stride < width * 2)
            throw std::invalid_argument("Invalid YUYV XFB frame");
        if (std::size_t(width) * 4 > SIZE_MAX / height || std::size_t(stride) > SIZE_MAX / height)
            throw std::overflow_error("XFB frame byte count overflow");
        std::vector<std::uint8_t> rgba(std::size_t(width) * height * 4);
        auto clamp = [](int value) { return static_cast<std::uint8_t>(std::clamp(value, 0, 255)); };
        for (unsigned y = 0; y < height; ++y) {
            const auto* row = source + std::size_t(y) * stride;
            for (unsigned x = 0; x < width; x += 2) {
                const int u = int(row[x * 2 + 1]) - 128;
                const int v = int(row[x * 2 + 3]) - 128;
                for (unsigned p = 0; p < 2; ++p) {
                    const int c = std::max(0, int(row[x * 2 + p * 2]) - 16);
                    auto* pixel = rgba.data() + (std::size_t(y) * width + x + p) * 4;
                    pixel[0] = clamp((298 * c + 409 * v + 128) >> 8);
                    pixel[1] = clamp((298 * c - 100 * u - 208 * v + 128) >> 8);
                    pixel[2] = clamp((298 * c + 516 * u + 128) >> 8);
                    pixel[3] = 255;
                }
            }
        }
        return rgba;
    }

    void renderYuyv(const std::uint8_t* source, unsigned width, unsigned height, unsigned stride) {
        auto rgba = decodeYuyv(source, width, height, stride);
        if (width != frameWidth || height != frameHeight) createFrame(width, height);
        context->UpdateSubresource(frame.Get(), 0, nullptr, rgba.data(), width * 4, 0);
        drawFrame();
    }

    void presentYuyv(const std::uint8_t* source, unsigned width, unsigned height, unsigned stride) {
        renderYuyv(source, width, height, stride);
        presentFrame();
    }

    // Readbacks must happen after rendering and before this call: the discard
    // swap chain does not preserve backbuffer contents after Present.
    void presentFrame() {
        RECT client{};
        if (!GetClientRect(window, &client) || client.right <= client.left || client.bottom <= client.top) return;
        check(swap->Present(0, 0), "Present game XFB frame");
    }

    std::vector<std::uint8_t> readbackBackbuffer() {
        if (!windowTarget || !targetWidth || !targetHeight) throw std::logic_error("No rendered XFB back buffer");
        NativeD3dComPtr<ID3D11Texture2D> backBuffer, staging;
        check(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())), "Get game back buffer for readback");
        D3D11_TEXTURE2D_DESC desc{}; backBuffer->GetDesc(&desc);
        desc.Usage = D3D11_USAGE_STAGING; desc.BindFlags = 0; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        check(device->CreateTexture2D(&desc, nullptr, &staging), "Create XFB present readback");
        context->CopyResource(staging.Get(), backBuffer.Get());
        D3D11_MAPPED_SUBRESOURCE mapped{};
        check(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped), "Map XFB present readback");
        std::vector<std::uint8_t> pixels(std::size_t(desc.Width) * desc.Height * 4);
        for (unsigned y = 0; y < desc.Height; ++y)
            std::memcpy(pixels.data() + std::size_t(y) * desc.Width * 4,
                        static_cast<const std::uint8_t*>(mapped.pData) + std::size_t(y) * mapped.RowPitch,
                        std::size_t(desc.Width) * 4);
        context->Unmap(staging.Get(), 0);
        return pixels;
    }
};
