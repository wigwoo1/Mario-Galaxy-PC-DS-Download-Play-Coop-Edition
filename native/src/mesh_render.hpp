#pragma once
#include "skinning.hpp"
#include "preview.hpp"
#include "materials.hpp"
struct RenderVertex {std::array<float,3> position,normal;std::array<float,2> uv{};std::array<float,4> color{.78f,.81f,.87f,1};int texture=-1;};
struct RetainedMeshAppearance {
    ModelMaterials materials;std::vector<std::vector<VertexAppearance>> vertices;
    RetainedMeshAppearance(BinaryView file,const ModelInfo& model,const ModelMesh& mesh):materials(decodeMaterials(file,model)){auto sections=modelSections(file);VertexArrays arrays(sections.at("VTX1"));for(const auto& packet:mesh.packets){std::vector<VertexAppearance> values;values.reserve(packet.vertices.size());for(const auto& vertex:packet.vertices)values.push_back(arrays.read(vertex));vertices.push_back(std::move(values));}}
};
inline std::vector<RenderVertex> defaultPoseTriangles(const ModelInfo& model,const ModelRig& rig,const ModelMesh& mesh){
    auto matrices=skinDrawMatrices(rig);std::vector<RenderVertex> vertices;vertices.reserve(mesh.triangles*3);
    for(const auto& packet:mesh.packets){auto positions=skinPacket(model,packet,matrices);
        for(auto t:packet.triangles){for(auto index:t)assetCheck(index<positions.size(),"Triangle index outside packet");
            auto a=positions[t[0]],b=positions[t[1]],c=positions[t[2]];std::array<float,3> u{},v{},n{};
            for(size_t k=0;k<3;++k){u[k]=b[k]-a[k];v[k]=c[k]-a[k];}
            n={u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]};
            float length=std::sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);if(length<1e-9f)continue;for(float& f:n)f/=length;
            vertices.push_back({a,n});vertices.push_back({b,n});vertices.push_back({c,n});
        }
    }
    assetCheck(!vertices.empty()&&vertices.size()<=10000000,"Empty or oversized render geometry");return vertices;
}
inline std::array<float,3> skinNormal(const AffineMatrix& m,const std::array<float,3>& n,const std::array<float,3>& fallback){
    float a=m[0],b=m[1],c=m[2],d=m[4],e=m[5],f=m[6],g=m[8],h=m[9],i=m[10];
    float determinant=a*(e*i-f*h)+b*(f*g-d*i)+c*(d*h-e*g);if(std::abs(determinant)<1e-12f)return fallback;
    std::array<float,3> result{((e*i-f*h)*n[0]+(f*g-d*i)*n[1]+(d*h-e*g)*n[2])/determinant,
        ((c*h-b*i)*n[0]+(a*i-c*g)*n[1]+(b*g-a*h)*n[2])/determinant,
        ((b*f-c*e)*n[0]+(c*d-a*f)*n[1]+(a*e-b*d)*n[2])/determinant};
    float length=std::sqrt(result[0]*result[0]+result[1]*result[1]+result[2]*result[2]);if(!std::isfinite(length)||length<1e-12f)return fallback;
    for(float& v:result)v/=length;return result;
}
inline std::vector<RenderVertex> materialPoseTriangles(BinaryView file,const ModelInfo& model,const ModelRig& rig,const ModelMesh& mesh,const ModelMaterials* retainedMaterials=nullptr,const std::vector<std::vector<VertexAppearance>>* retainedAppearance=nullptr){
    std::optional<VertexArrays> arrays;if(!retainedAppearance){auto sections=modelSections(file);arrays.emplace(sections.at("VTX1"));}ModelMaterials decoded;if(!retainedMaterials)decoded=decodeMaterials(file,model);const auto& materials=retainedMaterials?*retainedMaterials:decoded;auto matrices=skinDrawMatrices(rig);std::vector<RenderVertex> vertices;
    size_t packetIndex=0;if(retainedAppearance)assetCheck(retainedAppearance->size()==mesh.packets.size(),"Cached appearance packet count mismatch");for(const auto& packet:mesh.packets){const std::vector<VertexAppearance>* cachedAppearance=retainedAppearance?&retainedAppearance->at(packetIndex++):nullptr;if(cachedAppearance)assetCheck(cachedAppearance->size()==packet.vertices.size(),"Cached appearance vertex count mismatch");assetCheck(packet.shape<materials.shapeMaterial.size(),"Invalid packet material shape");auto material=materials.materials.at(materials.shapeMaterial[packet.shape]);auto positions=skinPacket(model,packet,matrices);
        for(auto triangle:packet.triangles){auto a=positions.at(triangle[0]),b=positions.at(triangle[1]),c=positions.at(triangle[2]);std::array<float,3> u{},v{},normal{};
            for(size_t k=0;k<3;++k){u[k]=b[k]-a[k];v[k]=c[k]-a[k];}normal={u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]};
            float length=std::sqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2]);if(length<1e-9f)continue;for(float& x:normal)x/=length;
            for(uint32_t index:triangle){auto vertex=packet.vertices.at(index);auto appearance=cachedAppearance?cachedAppearance->at(index):arrays->read(vertex);Pixel vc=appearance.colors[0];RenderVertex output{positions[index],skinNormal(matrices.at(vertex.drawMatrix),appearance.normal,normal)};output.uv=appearance.uv[0];
                output.color={material.color.r*vc.r/(255.0f*255),material.color.g*vc.g/(255.0f*255),material.color.b*vc.b/(255.0f*255),material.color.a*vc.a/(255.0f*255)};
                if(material.textures[0]!=0xffff&&packet.vertices[index].attributeIndices[13]!=UINT32_MAX)output.texture=material.textures[0];vertices.push_back(output);}
        }
    }
    assetCheck(!vertices.empty(),"Empty material geometry");return vertices;
}
struct MeshRenderCache {
    ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> context;
    ComPtr<ID3DBlob> vs,ps;
    ComPtr<ID3D11VertexShader> vertexShader;ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> layout;ComPtr<ID3D11DepthStencilState> depthState;ComPtr<ID3D11RasterizerState> rasterizer;
    ComPtr<ID3D11Texture2D> color,depthTexture,staging;ComPtr<ID3D11RenderTargetView> colorView;ComPtr<ID3D11DepthStencilView> depthView;
    uint32_t targetWidth=0,targetHeight=0;
    ComPtr<ID3D11Buffer> vertexBuffer;size_t vertexCapacity=0;
    std::vector<ComPtr<ID3D11ShaderResourceView>> resources;
    using SamplerKey=std::tuple<uint8_t,uint8_t,uint8_t,uint8_t,int8_t,int8_t,int16_t>;
    std::map<SamplerKey,ComPtr<ID3D11SamplerState>> samplers;
    const std::vector<TextureImage>* source=nullptr;bool texturesLoaded=false;
};
struct MeshCameraBounds {
    std::array<float,3> low{INFINITY,INFINITY,INFINITY},high{-INFINITY,-INFINITY,-INFINITY};
    void include(const std::vector<RenderVertex>& vertices){
        const float sine=std::sin(.35f),cosine=std::cos(.35f);
        for(const auto& v:vertices){auto p=v.position;float x=p[0],z=p[2];p[0]=cosine*x-sine*z;p[2]=sine*x+cosine*z;
            for(size_t k=0;k<3;++k){assetCheck(std::isfinite(p[k]),"Invalid camera position");low[k]=std::min(low[k],p[k]);high[k]=std::max(high[k],p[k]);}}
    }
};
inline TextureImage renderMesh(const std::vector<RenderVertex>& input,bool software,uint32_t width=720,uint32_t height=720,const std::vector<TextureImage>* textures=nullptr,MeshRenderCache* retained=nullptr,const MeshCameraBounds* camera=nullptr){
    assetCheck(!input.empty()&&width>0&&height>0&&width<=8192&&height<=8192,"Invalid mesh render target");
    ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> ctx;D3D_FEATURE_LEVEL feature=D3D_FEATURE_LEVEL_11_0;
    MeshRenderCache temporary;auto& cache=retained?*retained:temporary;
    if(!cache.device)dxCheck(D3D11CreateDevice(nullptr,software?D3D_DRIVER_TYPE_WARP:D3D_DRIVER_TYPE_HARDWARE,nullptr,0,&feature,1,D3D11_SDK_VERSION,&cache.device,nullptr,&cache.context));
    device=cache.device;ctx=cache.context;
    // Diagnostic orthographic camera. It frames the real CPU-skinned geometry.
    auto vertices=input;std::array<float,3> lo{INFINITY,INFINITY,INFINITY},hi{-INFINITY,-INFINITY,-INFINITY};
    const float sine=std::sin(0.35f),cosine=std::cos(0.35f);
    for(auto& v:vertices){float x=v.position[0],z=v.position[2];v.position[0]=cosine*x-sine*z;v.position[2]=sine*x+cosine*z;
        x=v.normal[0];z=v.normal[2];v.normal[0]=cosine*x-sine*z;v.normal[2]=sine*x+cosine*z;
        for(size_t k=0;k<3;++k){lo[k]=std::min(lo[k],v.position[k]);hi[k]=std::max(hi[k],v.position[k]);}}
    if(camera){lo=camera->low;hi=camera->high;for(size_t k=0;k<3;++k)assetCheck(std::isfinite(lo[k])&&std::isfinite(hi[k])&&hi[k]>=lo[k],"Invalid stable camera bounds");}
    float aspect=float(width)/height;float radius=std::max((hi[0]-lo[0])/aspect,hi[1]-lo[1])*0.6f;assetCheck(radius>0,"Degenerate mesh bounds");
    float depth=std::max(hi[2]-lo[2],1.0f);
    for(auto& v:vertices){v.position[0]=(v.position[0]-(lo[0]+hi[0])/2)/(radius*aspect);v.position[1]=(v.position[1]-(lo[1]+hi[1])/2)/radius;
        v.position[2]=0.1f+0.8f*(hi[2]-v.position[2])/depth;}
    const char* shader=R"(
Texture2D diffuse:register(t0);SamplerState sampleState:register(s0);
struct Input { float3 position:POSITION; float3 normal:NORMAL;float2 uv:TEXCOORD0;float4 color:COLOR;int texSlot:TEXCOORD1; };
struct Output { float4 position:SV_Position; float3 normal:NORMAL;float2 uv:TEXCOORD0;float4 color:COLOR;nointerpolation int texSlot:TEXCOORD1; };
Output vs(Input i){Output o;o.position=float4(i.position,1);o.normal=i.normal;o.uv=i.uv;o.color=i.color;o.texSlot=i.texSlot;return o;}
float4 ps(Output i):SV_Target{float shade=0.28+0.72*abs(dot(normalize(i.normal),normalize(float3(.3,.5,.8))));float4 color=i.color;if(i.texSlot>=0)color*=diffuse.Sample(sampleState,i.uv);clip(color.a-.05);return float4(color.rgb*shade,1);}
)";
    auto compile=[&](const char* entry,const char* target){ComPtr<ID3DBlob> code,errors;HRESULT hr=D3DCompile(shader,std::strlen(shader),"native-mesh-preview",nullptr,nullptr,entry,target,D3DCOMPILE_ENABLE_STRICTNESS,0,&code,&errors);
        if(FAILED(hr))throw std::runtime_error(errors?std::string(static_cast<const char*>(errors->GetBufferPointer()),errors->GetBufferSize()):"Mesh shader compile failure");return code;};
    if(!cache.vs){cache.vs=compile("vs","vs_4_0");cache.ps=compile("ps","ps_4_0");}
    auto vs=cache.vs,ps=cache.ps;auto& vertexShader=cache.vertexShader;auto& pixelShader=cache.pixelShader;
    if(!vertexShader)dxCheck(device->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),nullptr,&vertexShader));if(!pixelShader)dxCheck(device->CreatePixelShader(ps->GetBufferPointer(),ps->GetBufferSize(),nullptr,&pixelShader));
    D3D11_INPUT_ELEMENT_DESC elements[]={{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0},{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,32,D3D11_INPUT_PER_VERTEX_DATA,0},{"TEXCOORD",1,DXGI_FORMAT_R32_SINT,0,48,D3D11_INPUT_PER_VERTEX_DATA,0}};
    static_assert(sizeof(RenderVertex)==52);auto& layout=cache.layout;if(!layout)dxCheck(device->CreateInputLayout(elements,5,vs->GetBufferPointer(),vs->GetBufferSize(),&layout));
    assetCheck(vertices.size()<=UINT32_MAX/sizeof(RenderVertex),"Vertex buffer byte size exceeds D3D limit");size_t vertexBytes=vertices.size()*sizeof(RenderVertex);
    if(vertexBytes>cache.vertexCapacity){size_t capacity=256;while(capacity<vertexBytes){if(capacity>UINT32_MAX/2){capacity=vertexBytes;break;}capacity*=2;}
        D3D11_BUFFER_DESC bd{};bd.ByteWidth=UINT(capacity);bd.Usage=D3D11_USAGE_DYNAMIC;bd.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;bd.BindFlags=D3D11_BIND_VERTEX_BUFFER;ComPtr<ID3D11Buffer> replacement;dxCheck(device->CreateBuffer(&bd,nullptr,&replacement));cache.vertexBuffer=replacement;cache.vertexCapacity=capacity;}
    auto& buffer=cache.vertexBuffer;D3D11_MAPPED_SUBRESOURCE vertexMapping{};dxCheck(ctx->Map(buffer.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&vertexMapping));std::memcpy(vertexMapping.pData,vertices.data(),vertexBytes);ctx->Unmap(buffer.Get(),0);
    D3D11_TEXTURE2D_DESC td{};td.Width=width;td.Height=height;td.MipLevels=1;td.ArraySize=1;td.Format=DXGI_FORMAT_R8G8B8A8_UNORM;td.SampleDesc.Count=1;td.BindFlags=D3D11_BIND_RENDER_TARGET;
    if(cache.targetWidth!=width||cache.targetHeight!=height){ctx->OMSetRenderTargets(0,nullptr,nullptr);cache.color.Reset();cache.depthTexture.Reset();cache.staging.Reset();cache.colorView.Reset();cache.depthView.Reset();cache.targetWidth=cache.targetHeight=0;}
    auto& color=cache.color;auto& depthTexture=cache.depthTexture;auto& staging=cache.staging;auto& colorView=cache.colorView;auto& depthView=cache.depthView;
    if(!color)dxCheck(device->CreateTexture2D(&td,nullptr,&color));if(!colorView)dxCheck(device->CreateRenderTargetView(color.Get(),nullptr,&colorView));
    td.Format=DXGI_FORMAT_D32_FLOAT;td.BindFlags=D3D11_BIND_DEPTH_STENCIL;if(!depthTexture)dxCheck(device->CreateTexture2D(&td,nullptr,&depthTexture));if(!depthView)dxCheck(device->CreateDepthStencilView(depthTexture.Get(),nullptr,&depthView));
    td.Format=DXGI_FORMAT_R8G8B8A8_UNORM;td.BindFlags=0;td.Usage=D3D11_USAGE_STAGING;td.CPUAccessFlags=D3D11_CPU_ACCESS_READ;if(!staging)dxCheck(device->CreateTexture2D(&td,nullptr,&staging));cache.targetWidth=width;cache.targetHeight=height;
    D3D11_DEPTH_STENCIL_DESC ds{};ds.DepthEnable=TRUE;ds.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;ds.DepthFunc=D3D11_COMPARISON_LESS;
    auto& depthState=cache.depthState;if(!depthState)dxCheck(device->CreateDepthStencilState(&ds,&depthState));
    D3D11_RASTERIZER_DESC rs{};rs.FillMode=D3D11_FILL_SOLID;rs.CullMode=D3D11_CULL_NONE;rs.DepthClipEnable=TRUE;
    auto& rasterizer=cache.rasterizer;if(!rasterizer)dxCheck(device->CreateRasterizerState(&rs,&rasterizer));ctx->RSSetState(rasterizer.Get());
    float bg[]={24.0f/255,28.0f/255,37.0f/255,1};ctx->ClearRenderTargetView(colorView.Get(),bg);ctx->ClearDepthStencilView(depthView.Get(),D3D11_CLEAR_DEPTH,1,0);
    auto rtv=colorView.Get();ctx->OMSetRenderTargets(1,&rtv,depthView.Get());ctx->OMSetDepthStencilState(depthState.Get(),0);
    D3D11_VIEWPORT vp{0,0,float(width),float(height),0,1};ctx->RSSetViewports(1,&vp);
    UINT stride=sizeof(RenderVertex),offset=0;auto vb=buffer.Get();ctx->IASetVertexBuffers(0,1,&vb,&stride,&offset);ctx->IASetInputLayout(layout.Get());ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    if(cache.texturesLoaded)assetCheck(cache.source==textures,"Retained renderer texture set changed");
    auto& resources=cache.resources;
    if(!cache.texturesLoaded&&textures)for(const auto& texture:*textures){D3D11_TEXTURE2D_DESC desc{};desc.Width=texture.width;desc.Height=texture.height;desc.MipLevels=UINT(1+texture.mipmaps.size());desc.ArraySize=1;desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_IMMUTABLE;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        std::vector<D3D11_SUBRESOURCE_DATA> data{{texture.pixels.data(),texture.width*4,0}};uint32_t w=texture.width,h=texture.height;for(const auto& mip:texture.mipmaps){w=std::max(1u,w/2);h=std::max(1u,h/2);assetCheck(mip.width==w&&mip.height==h&&mip.pixels.size()==size_t(w)*h,"Invalid render mip dimensions");data.push_back({mip.pixels.data(),w*4,0});}ComPtr<ID3D11Texture2D> image;ComPtr<ID3D11ShaderResourceView> resource;dxCheck(device->CreateTexture2D(&desc,data.data(),&image));dxCheck(device->CreateShaderResourceView(image.Get(),nullptr,&resource));resources.push_back(resource);}
    cache.source=textures;cache.texturesLoaded=true;
    auto& samplers=cache.samplers;
    auto address=[](uint8_t mode){assetCheck(mode<=2,"Invalid render wrap mode");return mode==0?D3D11_TEXTURE_ADDRESS_CLAMP:mode==1?D3D11_TEXTURE_ADDRESS_WRAP:D3D11_TEXTURE_ADDRESS_MIRROR;};
    ctx->VSSetShader(vertexShader.Get(),nullptr,0);ctx->PSSetShader(pixelShader.Get(),nullptr,0);
    for(size_t first=0;first<vertices.size();){int texture=vertices[first].texture;size_t end=first+3;assetCheck(end<=vertices.size(),"Incomplete render triangle");
        while(end<vertices.size()&&vertices[end].texture==texture)end+=3;assetCheck(end<=vertices.size(),"Incomplete render batch");
        ID3D11ShaderResourceView* srv=nullptr;if(texture>=0){assetCheck(size_t(texture)<resources.size(),"Missing render texture");srv=resources[size_t(texture)].Get();}
        uint8_t wrapS=1,wrapT=1,minFilter=1,magFilter=1;int8_t minLod=0,maxLod=127;int16_t bias=0;if(texture>=0){auto& image=(*textures)[size_t(texture)];wrapS=image.wrapS;wrapT=image.wrapT;minFilter=image.minFilter;magFilter=image.magFilter;minLod=image.minLod;maxLod=image.maxLod;bias=image.lodBias;}address(wrapS);address(wrapT);assetCheck(minFilter<=5&&magFilter<=1&&minLod<=maxLod,"Invalid render texture filter/LOD");auto& sampler=samplers[{wrapS,wrapT,minFilter,magFilter,minLod,maxLod,bias}];
        if(!sampler){D3D11_SAMPLER_DESC desc{};desc.Filter=D3D11_FILTER(((minFilter&1)?0x10:0)|(magFilter?0x4:0)|(minFilter>=4?1:0));desc.AddressU=address(wrapS);desc.AddressV=address(wrapT);desc.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;desc.MinLOD=minFilter<2?0:minLod/8.f;desc.MaxLOD=minFilter<2?0:maxLod/8.f;desc.MipLODBias=std::clamp(bias/100.f,-16.f,15.99f);dxCheck(device->CreateSamplerState(&desc,&sampler));}auto samplerPtr=sampler.Get();ctx->PSSetSamplers(0,1,&samplerPtr);
        ctx->PSSetShaderResources(0,1,&srv);ctx->Draw(UINT(end-first),UINT(first));first=end;}
    ctx->OMSetRenderTargets(0,nullptr,nullptr);
    td.Format=DXGI_FORMAT_R8G8B8A8_UNORM;td.BindFlags=0;td.Usage=D3D11_USAGE_STAGING;td.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    ctx->CopyResource(staging.Get(),color.Get());D3D11_MAPPED_SUBRESOURCE mapped{};
    TextureImage image{width,height,6,std::vector<Pixel>(size_t(width)*height)};dxCheck(ctx->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped));
    for(uint32_t y=0;y<height;++y)std::memcpy(image.pixels.data()+size_t(y)*width,static_cast<uint8_t*>(mapped.pData)+size_t(y)*mapped.RowPitch,width*4);
    ctx->Unmap(staging.Get(),0);return image;
}
inline void meshRenderTests(){
    auto scaled=identityMatrix();scaled[0]=2;scaled[5]=3;scaled[10]=4;auto normal=skinNormal(scaled,{1,1,0},{0,0,1});
    assetCheck(std::abs(normal[0]-0.8320503f)<0.0001&&std::abs(normal[1]-0.5547002f)<0.0001,"Inverse-transpose normal test failed");
    // Two overlapping triangles: near red-normal shade must win regardless of order.
    std::vector<RenderVertex> geometry={{{-1,-1,1},{0,0,1}},{{1,-1,1},{0,0,1}},{{0,1,1},{0,0,1}},
        {{-1,-1,-1},{0,1,0}},{{1,-1,-1},{0,1,0}},{{0,1,-1},{0,1,0}}};
    auto a=renderMesh(geometry,true,64,64);std::rotate(geometry.begin(),geometry.begin()+3,geometry.end());auto b=renderMesh(geometry,true,64,64);
    assetCheck(a.pixels.size()==b.pixels.size(),"Mesh render dimensions differ");
    size_t changed=0;for(size_t i=0;i<a.pixels.size();++i){assetCheck(same(a.pixels[i],b.pixels[i]),"Depth rendering depends on triangle order");if(a.pixels[i].r!=24||a.pixels[i].g!=28||a.pixels[i].b!=37)++changed;}
    assetCheck(changed>100,"Mesh render produced no geometry");
    for(auto& vertex:geometry){vertex.texture=0;vertex.color={1,1,1,1};}
    std::vector<TextureImage> textures{{1,1,6,{{255,0,0,255}}}};auto textured=renderMesh(geometry,true,64,64,&textures);
    Pixel center=textured.pixels[32*64+32];assetCheck(center.r>50&&center.g==0&&center.b==0,"Mesh texture sampling test failed");
    textures[0].pixels[0].a=0;auto transparent=renderMesh(geometry,true,64,64,&textures);
    for(Pixel p:transparent.pixels)assetCheck(p.r==24&&p.g==28&&p.b==37,"Mesh alpha discard test failed");
    textures[0]={2,1,6,{{255,0,0,255},{0,255,0,255}}};for(auto& vertex:geometry)vertex.uv={1.25f,.5f};
    textures[0].wrapS=0;auto clamped=renderMesh(geometry,true,64,64,&textures);textures[0].wrapS=1;auto repeated=renderMesh(geometry,true,64,64,&textures);textures[0].wrapS=2;auto mirrored=renderMesh(geometry,true,64,64,&textures);
    auto c=clamped.pixels[32*64+32],r=repeated.pixels[32*64+32],m=mirrored.pixels[32*64+32];assetCheck(c.g>50&&c.r==0&&r.r>50&&r.g==0&&m.g>50&&m.r==0,"GX clamp/repeat/mirror sampling failed");
    MeshRenderCache retained;renderMesh(geometry,true,64,64,&textures,&retained);auto device=retained.device.Get();auto sampler=retained.samplers.begin()->second.Get();assetCheck(sampler!=nullptr,"Retained mirror sampler missing");
    auto vertexShader=retained.vertexShader.Get();auto pixelShader=retained.pixelShader.Get();auto layout=retained.layout.Get();auto depthState=retained.depthState.Get();auto rasterizer=retained.rasterizer.Get();
    auto color=retained.color.Get(),depth=retained.depthTexture.Get(),staging=retained.staging.Get();
    auto vertexBuffer=retained.vertexBuffer.Get();auto vertexCapacity=retained.vertexCapacity;
    auto again=renderMesh(geometry,true,64,64,&textures,&retained);assetCheck(retained.device.Get()==device&&retained.samplers.begin()->second.Get()==sampler,"GPU texture sampler was recreated");
    assetCheck(vertexShader&&pixelShader&&layout&&depthState&&rasterizer&&retained.vertexShader.Get()==vertexShader&&retained.pixelShader.Get()==pixelShader&&retained.layout.Get()==layout&&retained.depthState.Get()==depthState&&retained.rasterizer.Get()==rasterizer,"GPU pipeline state was recreated");
    for(size_t i=0;i<again.pixels.size();++i)assetCheck(same(again.pixels[i],mirrored.pixels[i]),"Retained pipeline changed rendering");
    assetCheck(retained.color.Get()==color&&retained.depthTexture.Get()==depth&&retained.staging.Get()==staging,"Render targets recreated at same size");
    assetCheck(retained.vertexBuffer.Get()==vertexBuffer&&retained.vertexCapacity==vertexCapacity,"Same-size dynamic vertex buffer recreated");
    auto resized=renderMesh(geometry,true,96,48,&textures,&retained);auto resizeReference=renderMesh(geometry,true,96,48,&textures);assetCheck(retained.targetWidth==96&&retained.targetHeight==48&&retained.device.Get()==device&&retained.vertexShader.Get()==vertexShader,"Render target resize changed device/pipeline");
    for(size_t i=0;i<resized.pixels.size();++i)assetCheck(same(resized.pixels[i],resizeReference.pixels[i]),"Resized retained render differs");
    auto restored=renderMesh(geometry,true,64,64,&textures,&retained);for(size_t i=0;i<restored.pixels.size();++i)assetCheck(same(restored.pixels[i],mirrored.pixels[i]),"Render target resize back differs");
    auto updated=geometry;for(auto& vertex:updated)vertex.uv={.25f,.5f};auto updateImage=renderMesh(updated,true,64,64,&textures,&retained);auto updatePixel=updateImage.pixels[32*64+32];assetCheck(updatePixel.r>50&&updatePixel.g==0&&retained.vertexBuffer.Get()==vertexBuffer,"Dynamic vertex upload retained stale data");
    std::vector<RenderVertex> grown;for(int i=0;i<4;++i)grown.insert(grown.end(),geometry.begin(),geometry.end());auto grownImage=renderMesh(grown,true,64,64,&textures,&retained);assetCheck(retained.vertexCapacity>vertexCapacity&&retained.vertexCapacity>=grown.size()*sizeof(RenderVertex),"Dynamic vertex capacity did not grow");
    for(size_t i=0;i<grownImage.pixels.size();++i)assetCheck(same(grownImage.pixels[i],mirrored.pixels[i]),"Grown dynamic vertex buffer changed rendering");
    auto grownBuffer=retained.vertexBuffer.Get();renderMesh(geometry,true,64,64,&textures,&retained);assetCheck(retained.vertexBuffer.Get()==grownBuffer,"Smaller draw discarded dynamic buffer capacity");
    for(auto& vertex:geometry)vertex.uv={.4f,.5f};textures[0].minFilter=textures[0].magFilter=0;auto point=renderMesh(geometry,true,64,64,&textures);textures[0].minFilter=textures[0].magFilter=1;auto linear=renderMesh(geometry,true,64,64,&textures);auto p=point.pixels[32*64+32],l=linear.pixels[32*64+32];assetCheck(p.r>50&&p.g==0&&l.r>50&&l.g>20,"GX nearest/linear sampling failed");
    textures[0]={4,4,6,std::vector<Pixel>(16,{255,0,0,255})};textures[0].mipmaps={{2,2,std::vector<Pixel>(4,{0,255,0,255})},{1,1,{{0,0,255,255}}}};textures[0].minFilter=2;textures[0].magFilter=0;for(auto& vertex:geometry)vertex.uv={(vertex.position[0]+1)*32,(vertex.position[1]+1)*32};auto mipImage=renderMesh(geometry,true,64,64,&textures);auto mipPixel=mipImage.pixels[32*64+32];assetCheck(mipPixel.b>50&&mipPixel.r==0&&mipPixel.g==0,"GPU did not sample stored lower mip");
    textures[0].minFilter=0;auto baseImage=renderMesh(geometry,true,64,64,&textures);auto basePixel=baseImage.pixels[32*64+32];assetCheck(basePixel.r>50&&basePixel.b==0,"Non-mip GX filter sampled lower mip");
    textures[0].minFilter=2;textures[0].minLod=textures[0].maxLod=8;auto limited=renderMesh(geometry,true,64,64,&textures);auto limitedPixel=limited.pixels[32*64+32];assetCheck(limitedPixel.g>50&&limitedPixel.r==0&&limitedPixel.b==0,"Stored LOD clamp did not select requested level");
}





