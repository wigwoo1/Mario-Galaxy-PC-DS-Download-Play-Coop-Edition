#pragma once
#include "layout_resource.hpp"
// Floating-point adaptation of the default one-texture nw4r TEV path.
// It does not reproduce GX fixed-point rounding or authored explicit TEV stages.
inline std::array<float,4> shadeLayoutTexture(const NativeLayoutMaterial& material,const std::array<float,4>& sample,uint32_t vertexColor,float paneAlpha){
    assetCheck((material.resourceBits&~0xFFFu)==0&&material.textures.size()==1&&(material.resourceBits&15)==1,"Unsupported layout material shading configuration");
    assetCheck(std::isfinite(paneAlpha)&&paneAlpha>=0&&paneAlpha<=255,"Invalid layout shading pane alpha");
    for(float channel:sample)assetCheck(std::isfinite(channel)&&channel>=0&&channel<=255,"Invalid layout texture sample");std::array<float,4> result{};
    for(unsigned i=0;i<4;++i){float t=sample[i]/255.0f;float stage=std::clamp(material.tevColors[0][i]*(1-t)+material.tevColors[1][i]*t,0.0f,255.0f);float raster=float((vertexColor>>(24-i*8))&255);if(i==3)raster*=paneAlpha/255.0f;result[i]=stage*raster/255.0f;}return result;
}
inline std::array<float,4> shadeLayoutTexel(const NativeLayoutMaterial& material,Pixel texel,uint32_t vertexColor,float paneAlpha){return shadeLayoutTexture(material,{float(texel.r),float(texel.g),float(texel.b),float(texel.a)},vertexColor,paneAlpha);}
inline void layoutMaterialShadingTests(){
    NativeLayoutMaterial material;material.resourceBits=1;material.textures={{0,0,0}};material.tevColors[0]={0,0,0,0};material.tevColors[1]={255,255,255,255};
    auto value=shadeLayoutTexture(material,{64,128,192,255},0xFFFFFFFF,128);assetCheck(value==std::array<float,4>{64,128,192,128},"Layout texture/raster modulation mismatch");
    material.tevColors[1]={0,0,0,255};value=shadeLayoutTexture(material,{128,128,128,128},0xFFFFFFFF,255);assetCheck(value==std::array<float,4>{0,0,0,128},"Layout black mask alpha mismatch");
    assetCheck(shadeLayoutTexture(material,{255,255,255,255},0xFFFFFFFF,0)[3]==0,"Layout pane alpha not applied");
    material.tevColors[0]={-100,-100,-100,-100};material.tevColors[1]={400,400,400,400};value=shadeLayoutTexture(material,{255,255,255,255},0x80808080,255);assetCheck(value==std::array<float,4>{128,128,128,128},"Layout stage clamp order mismatch");
    material.resourceBits|=0x1000;bool rejected=false;try{shadeLayoutTexture(material,{0,0,0,0},0xFFFFFFFF,255);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Unsupported layout shading silently accepted");
}

