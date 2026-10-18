#pragma once
#include "graphics.hpp"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <iostream>
using Microsoft::WRL::ComPtr;
inline void dxCheck(HRESULT hr) {
    if(FAILED(hr)) throw std::runtime_error("Direct3D operation failed: "+std::to_string(static_cast<uint32_t>(hr)));
}
class TextureRenderer {
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11ShaderResourceView> resource;
    ComPtr<ID3D11SamplerState> sampler;
    ComPtr<ID3D11RasterizerState> rasterizer;
    ComPtr<IDXGISwapChain> swap;
    ComPtr<ID3D11RenderTargetView> windowTarget;
    uint32_t textureWidth,textureHeight,windowWidth=0,windowHeight=0;
    void draw(ID3D11RenderTargetView* target,uint32_t w,uint32_t h) {
        float clear[4]={0.025f,0.03f,0.05f,1};context->ClearRenderTargetView(target,clear);
        context->OMSetRenderTargets(1,&target,nullptr);
        float scale=std::min(float(w)/textureWidth,float(h)/textureHeight);
        D3D11_VIEWPORT vp{(w-textureWidth*scale)/2,(h-textureHeight*scale)/2,textureWidth*scale,textureHeight*scale,0,1};
        context->RSSetViewports(1,&vp);context->RSSetState(rasterizer.Get());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(vertexShader.Get(),nullptr,0);context->PSSetShader(pixelShader.Get(),nullptr,0);
        auto srv=resource.Get();auto smp=sampler.Get();context->PSSetShaderResources(0,1,&srv);context->PSSetSamplers(0,1,&smp);
        context->Draw(3,0);
    }
public:
    explicit TextureRenderer(const TextureImage& image,bool software=false):textureWidth(image.width),textureHeight(image.height) {
        D3D_FEATURE_LEVEL level=D3D_FEATURE_LEVEL_11_0;
        dxCheck(D3D11CreateDevice(nullptr,software?D3D_DRIVER_TYPE_WARP:D3D_DRIVER_TYPE_HARDWARE,nullptr,0,&level,1,D3D11_SDK_VERSION,&device,nullptr,&context));
        const char* shader=R"(
Texture2D source : register(t0);
SamplerState sampling : register(s0);
struct Varying { float4 position:SV_Position; float2 uv:TEXCOORD0; };
Varying vs(uint id:SV_VertexID) {
    Varying o; o.uv=float2((id<<1)&2,id&2);o.position=float4(o.uv.x*2-1,1-o.uv.y*2,0,1);return o;
}
float4 ps(Varying i):SV_Target {
    uint w,h;source.GetDimensions(w,h);uint2 p=min(uint2(i.uv*float2(w,h)),uint2(w-1,h-1));
    float background=(((p.x/16+p.y/16)&1)!=0?56.0:32.0)/255.0;
    float4 color=source.Sample(sampling,i.uv);
    return float4(lerp(background.xxx,color.rgb,color.a),1);
})";
        auto compile=[&](const char* entry,const char* profile) {
            ComPtr<ID3DBlob> code,errors;
            HRESULT result=D3DCompile(shader,std::strlen(shader),"native-texture-preview",nullptr,nullptr,entry,profile,D3DCOMPILE_ENABLE_STRICTNESS,0,&code,&errors);
            if(FAILED(result)) throw std::runtime_error(errors?std::string(static_cast<const char*>(errors->GetBufferPointer()),errors->GetBufferSize()):"Shader compile failed");
            return code;
        };
        auto vs=compile("vs","vs_4_0"),ps=compile("ps","ps_4_0");
        dxCheck(device->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),nullptr,&vertexShader));
        dxCheck(device->CreatePixelShader(ps->GetBufferPointer(),ps->GetBufferSize(),nullptr,&pixelShader));
        D3D11_TEXTURE2D_DESC desc{};desc.Width=image.width;desc.Height=image.height;desc.MipLevels=1;desc.ArraySize=1;
        desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        static_assert(sizeof(Pixel)==4);
        D3D11_SUBRESOURCE_DATA init{image.pixels.data(),image.width*4,0};ComPtr<ID3D11Texture2D> texture;
        dxCheck(device->CreateTexture2D(&desc,&init,&texture));dxCheck(device->CreateShaderResourceView(texture.Get(),nullptr,&resource));
        D3D11_SAMPLER_DESC s{};s.Filter=D3D11_FILTER_MIN_MAG_MIP_POINT;s.AddressU=s.AddressV=s.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;s.MaxLOD=D3D11_FLOAT32_MAX;
        dxCheck(device->CreateSamplerState(&s,&sampler));
        D3D11_RASTERIZER_DESC r{};r.FillMode=D3D11_FILL_SOLID;r.CullMode=D3D11_CULL_NONE;r.DepthClipEnable=TRUE;
        dxCheck(device->CreateRasterizerState(&r,&rasterizer));
    }
    TextureImage readback() {
        D3D11_TEXTURE2D_DESC d{};d.Width=textureWidth;d.Height=textureHeight;d.MipLevels=1;d.ArraySize=1;d.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        d.SampleDesc.Count=1;d.Usage=D3D11_USAGE_DEFAULT;d.BindFlags=D3D11_BIND_RENDER_TARGET;
        ComPtr<ID3D11Texture2D> output,staging;ComPtr<ID3D11RenderTargetView> target;
        dxCheck(device->CreateTexture2D(&d,nullptr,&output));dxCheck(device->CreateRenderTargetView(output.Get(),nullptr,&target));
        draw(target.Get(),textureWidth,textureHeight);context->OMSetRenderTargets(0,nullptr,nullptr);
        d.Usage=D3D11_USAGE_STAGING;d.BindFlags=0;d.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
        dxCheck(device->CreateTexture2D(&d,nullptr,&staging));context->CopyResource(staging.Get(),output.Get());
        TextureImage image{textureWidth,textureHeight,6,std::vector<Pixel>(size_t(textureWidth)*textureHeight)};
        D3D11_MAPPED_SUBRESOURCE mapped{};dxCheck(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped));
        for(uint32_t y=0;y<textureHeight;++y)std::memcpy(image.pixels.data()+size_t(y)*textureWidth,static_cast<uint8_t*>(mapped.pData)+size_t(y)*mapped.RowPitch,textureWidth*4);
        context->Unmap(staging.Get(),0);return image;
    }
    void updateImage(const TextureImage& image){
        assetCheck(image.width==textureWidth&&image.height==textureHeight&&image.pixels.size()==size_t(textureWidth)*textureHeight,"Preview update dimensions changed");
        ComPtr<ID3D11Resource> texture;resource->GetResource(&texture);
        context->UpdateSubresource(texture.Get(),0,nullptr,image.pixels.data(),textureWidth*4,0);
    }
    void attach(HWND window) {
        ComPtr<IDXGIDevice> dxgi;dxCheck(device.As(&dxgi));ComPtr<IDXGIAdapter> adapter;dxCheck(dxgi->GetAdapter(&adapter));
        ComPtr<IDXGIFactory> factory;dxCheck(adapter->GetParent(__uuidof(IDXGIFactory),reinterpret_cast<void**>(factory.GetAddressOf())));
        DXGI_SWAP_CHAIN_DESC d{};d.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;d.SampleDesc.Count=1;d.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
        d.BufferCount=2;d.OutputWindow=window;d.Windowed=TRUE;d.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
        dxCheck(factory->CreateSwapChain(device.Get(),&d,&swap));dxCheck(factory->MakeWindowAssociation(window,DXGI_MWA_NO_ALT_ENTER));
    }
    void present(uint32_t w,uint32_t h) {
        if(!w||!h)return;
        if(w!=windowWidth||h!=windowHeight) {
            context->OMSetRenderTargets(0,nullptr,nullptr);windowTarget.Reset();
            dxCheck(swap->ResizeBuffers(0,w,h,DXGI_FORMAT_UNKNOWN,0));ComPtr<ID3D11Texture2D> buffer;
            dxCheck(swap->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(buffer.GetAddressOf())));
            dxCheck(device->CreateRenderTargetView(buffer.Get(),nullptr,&windowTarget));windowWidth=w;windowHeight=h;
        }
        draw(windowTarget.Get(),w,h);dxCheck(swap->Present(1,0));
    }
};
inline TextureImage verifyRendering(const TextureImage& source,bool software) {
    TextureRenderer renderer(source,software);auto result=renderer.readback();
    for(uint32_t y=0;y<source.height;++y)for(uint32_t x=0;x<source.width;++x) {
        Pixel s=source.pixels[size_t(y)*source.width+x],r=result.pixels[size_t(y)*source.width+x];
        unsigned bg=((x/16+y/16)%2)?56:32;
        auto close=[&](unsigned original,unsigned actual){int expected=int((original*s.a+bg*(255-s.a)+127)/255);return std::abs(expected-int(actual))<=1;};
        assetCheck(close(s.r,r.r)&&close(s.g,r.g)&&close(s.b,r.b)&&r.a==255,"Direct3D readback pixel mismatch");
    }
    return result;
}
struct PreviewWindowState { TextureRenderer* renderer=nullptr;std::string error;unsigned frames=0; };
inline LRESULT CALLBACK previewProcedure(HWND window,UINT message,WPARAM wp,LPARAM lp) {
    auto state=reinterpret_cast<PreviewWindowState*>(GetWindowLongPtrW(window,GWLP_USERDATA));
    if(message==WM_NCCREATE) {auto cs=reinterpret_cast<CREATESTRUCTW*>(lp);SetWindowLongPtrW(window,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(cs->lpCreateParams));}
    if(message==WM_KEYDOWN&&wp==VK_ESCAPE) {DestroyWindow(window);return 0;}
    if(message==WM_SIZE) {InvalidateRect(window,nullptr,FALSE);return 0;}
    if(message==WM_PAINT) {
        PAINTSTRUCT paint;BeginPaint(window,&paint);
        if(state&&state->renderer) {
            RECT r{};GetClientRect(window,&r);
            try {state->renderer->present(r.right,r.bottom);++state->frames;}
            catch(const std::exception& e){state->error=e.what();}
        }
        EndPaint(window,&paint);if(state&&!state->error.empty())DestroyWindow(window);return 0;
    }
    if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
inline void previewTexture(const TextureImage& image,bool smoke=false) {
    std::unique_ptr<TextureRenderer> renderer;
    try {renderer=std::make_unique<TextureRenderer>(image);}catch(const std::exception&) {renderer=std::make_unique<TextureRenderer>(image,true);}
    auto instance=GetModuleHandleW(nullptr);WNDCLASSW wc{};wc.lpfnWndProc=previewProcedure;wc.hInstance=instance;
    wc.lpszClassName=L"GalaxyNativeTexturePreview";wc.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));
    assetCheck(RegisterClassW(&wc)!=0,"Cannot register preview window");
    PreviewWindowState state{renderer.get(),"",0};
    HWND window=CreateWindowExW(0,wc.lpszClassName,L"Galaxy native texture preview - not the game",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,900,600,nullptr,nullptr,instance,&state);
    if(!window){UnregisterClassW(wc.lpszClassName,instance);throw std::runtime_error("Cannot create preview window");}
    try {
        renderer->attach(window);
        if(smoke) {
            SendMessageW(window,WM_PAINT,0,0);
            SetWindowPos(window,nullptr,0,0,640,480,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
            SendMessageW(window,WM_PAINT,0,0);
            assetCheck(state.error.empty()&&state.frames==2,"Hidden preview paint/resize test failed");
            DestroyWindow(window);
        } else {
            ShowWindow(window,SW_SHOW);UpdateWindow(window);MSG message{};
            for(;;){BOOL value=GetMessageW(&message,nullptr,0,0);if(value==-1)throw std::runtime_error("Window message loop failed");if(value==0)break;TranslateMessage(&message);DispatchMessageW(&message);}
            if(!state.error.empty())throw std::runtime_error(state.error);
        }
    }catch(...){if(IsWindow(window))DestroyWindow(window);UnregisterClassW(wc.lpszClassName,instance);throw;}
    renderer.reset();UnregisterClassW(wc.lpszClassName,instance);
}
