#pragma once
#include "layout_material_shading.hpp"
// Base-level native sampling. Material wrapping overrides TPL wrapping.
inline std::array<float,4> sampleLayoutTexture(const TextureImage& texture,const NativeLayoutTextureMap& map,std::array<float,2> uv,bool minifying=false){
    assetCheck(texture.width&&texture.height&&texture.pixels.size()==uint64_t(texture.width)*texture.height,"Invalid layout sampler texture");assetCheck(map.wrapS<=2&&map.wrapT<=2&&texture.minFilter<=1&&texture.magFilter<=1&&texture.mipmaps.empty(),"Unsupported native layout sampler");
    auto coordinate=[](float value,uint8_t wrap){assetCheck(std::isfinite(value),"Invalid layout sample coordinate");if(!wrap)return std::clamp(double(value),0.0,1.0);double period=wrap==1?1:2;double result=std::fmod(double(value),period);if(result<0)result+=period;return result;};
    double u=coordinate(uv[0],map.wrapS),v=coordinate(uv[1],map.wrapT);
    auto index=[](int64_t value,uint32_t size,uint8_t wrap){if(!wrap)return uint32_t(std::clamp<int64_t>(value,0,size-1));int64_t period=int64_t(size)*(wrap==2?2:1);value%=period;if(value<0)value+=period;if(value>=size)value=period-1-value;return uint32_t(value);};
    auto read=[&](int64_t x,int64_t y){const auto& p=texture.pixels[size_t(index(y,texture.height,map.wrapT))*texture.width+index(x,texture.width,map.wrapS)];return std::array<float,4>{float(p.r),float(p.g),float(p.b),float(p.a)};};
    if(!(minifying?texture.minFilter:texture.magFilter))return read(int64_t(std::floor(u*texture.width)),int64_t(std::floor(v*texture.height)));
    double x=u*texture.width-.5,y=v*texture.height-.5;int64_t ix=int64_t(std::floor(x)),iy=int64_t(std::floor(y));float fx=float(x-ix),fy=float(y-iy);auto a=read(ix,iy),b=read(ix+1,iy),c=read(ix,iy+1),d=read(ix+1,iy+1);std::array<float,4> result{};for(unsigned i=0;i<4;++i)result[i]=(a[i]*(1-fx)+b[i]*fx)*(1-fy)+(c[i]*(1-fx)+d[i]*fx)*fy;return result;
}
inline void layoutTextureSamplingTests(){
    TextureImage texture{2,1,6,{{255,0,0,0},{0,0,255,255}}};NativeLayoutTextureMap map{0,0,0};texture.minFilter=texture.magFilter=0;
    assetCheck(sampleLayoutTexture(texture,map,{-1,.5f})==std::array<float,4>{255,0,0,0},"Layout clamp sample mismatch");map.wrapS=1;assetCheck(sampleLayoutTexture(texture,map,{-.25f,.5f})==std::array<float,4>{0,0,255,255},"Layout negative repeat sample mismatch");map.wrapS=2;assetCheck(sampleLayoutTexture(texture,map,{1.75f,.5f})==std::array<float,4>{255,0,0,0},"Layout mirrored sample mismatch");
    texture.magFilter=1;map.wrapS=1;auto seam=sampleLayoutTexture(texture,map,{0,.5f});assetCheck(seam==std::array<float,4>{127.5f,0,127.5f,127.5f},"Layout bilinear repeat seam mismatch");map.wrapS=0;assetCheck(sampleLayoutTexture(texture,map,{0,.5f})==std::array<float,4>{255,0,0,0},"Layout linear clamp edge mismatch");assetCheck(sampleLayoutTexture(texture,map,{.5f,.5f})==seam,"Layout linear midpoint mismatch");assetCheck(sampleLayoutTexture(texture,map,{.5f,.5f},true)==std::array<float,4>{0,0,255,255},"Layout minification filter selection mismatch");
}
