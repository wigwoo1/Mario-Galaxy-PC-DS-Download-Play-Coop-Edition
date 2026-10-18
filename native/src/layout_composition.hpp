#pragma once
#include "layout_texture_sampling.hpp"
#include "layout_texture_transform.hpp"
// Native CPU composition of affine 2D layout quads into an existing frame.
// Center-origin layout coordinates map to the full target; retail aspect adjustment is pending.
inline size_t compositeLayoutQuads(TextureImage& target,const NativeLayoutResource& pose,const std::vector<NativeLayoutQuad>& mesh,const std::vector<TextureImage>& textures){
    assetCheck(target.width&&target.height&&target.pixels.size()==uint64_t(target.width)*target.height,"Invalid layout composition target");assetCheck(pose.origin==1&&pose.size[0]>0&&pose.size[1]>0,"Unsupported layout composition viewport");
    size_t painted=0;
    for(const auto& quad:mesh){if(quad.alpha==0)continue;const auto& material=pose.materials.at(quad.material);assetCheck(material.textures.size()==1,"Unsupported layout composition texture count");const auto& map=material.textures.front();const auto& texture=textures.at(map.texture);
        std::array<std::array<double,2>,4> screen{};double minX=target.width,minY=target.height,maxX=0,maxY=0;
        for(unsigned i=0;i<4;++i){screen[i]={(quad.positions[i][0]/pose.size[0]+.5)*target.width,(.5-quad.positions[i][1]/pose.size[1])*target.height};minX=std::min(minX,screen[i][0]);maxX=std::max(maxX,screen[i][0]);minY=std::min(minY,screen[i][1]);maxY=std::max(maxY,screen[i][1]);}
        double ax=screen[1][0]-screen[0][0],ay=screen[1][1]-screen[0][1],bx=screen[2][0]-screen[0][0],by=screen[2][1]-screen[0][1],det=ax*by-ay*bx;if(std::abs(det)<1e-10)continue;
        int x0=int(std::floor(std::clamp(minX,0.0,double(target.width)))),x1=int(std::ceil(std::clamp(maxX,0.0,double(target.width)))),y0=int(std::floor(std::clamp(minY,0.0,double(target.height)))),y1=int(std::ceil(std::clamp(maxY,0.0,double(target.height))));
        double duDx=((quad.uv[2]-quad.uv[0])*by-(quad.uv[4]-quad.uv[0])*ay)/det,duDy=(-(quad.uv[2]-quad.uv[0])*bx+(quad.uv[4]-quad.uv[0])*ax)/det,dvDx=((quad.uv[3]-quad.uv[1])*by-(quad.uv[5]-quad.uv[1])*ay)/det,dvDy=(-(quad.uv[3]-quad.uv[1])*bx+(quad.uv[5]-quad.uv[1])*ax)/det;bool minifying=std::hypot(duDx*texture.width,dvDx*texture.height)>1||std::hypot(duDy*texture.width,dvDy*texture.height)>1;
        for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x){double dx=x+.5-screen[0][0],dy=y+.5-screen[0][1],u=(dx*by-dy*bx)/det,v=(ax*dy-ay*dx)/det;if(u<0||v<0||u>=1||v>=1)continue;
            std::array<float,4> weights{};if(u+v<=1)weights={float(1-u-v),float(u),float(v),0};else weights={0,float(1-v),float(1-u),float(u+v-1)};
            std::array<float,2> uv{};uint32_t vertexColor=0;for(unsigned i=0;i<4;++i){uv[0]+=weights[i]*quad.uv[i*2];uv[1]+=weights[i]*quad.uv[i*2+1];}for(unsigned channel=0;channel<4;++channel){float value=0;for(unsigned i=0;i<4;++i)value+=weights[i]*float((quad.colors[i]>>(24-channel*8))&255);vertexColor|=uint32_t(std::clamp(std::lround(value),0l,255l))<<(24-channel*8);}
            auto source=shadeLayoutTexture(material,sampleLayoutTexture(texture,map,uv,minifying),vertexColor,quad.alpha);if(source[3]==0)continue;auto& destination=target.pixels[size_t(y)*target.width+x];float alpha=source[3]/255;auto blend=[&](unsigned channel,uint8_t value){return uint8_t(std::clamp(std::lround(source[channel]*alpha+value*(1-alpha)),0l,255l));};destination={blend(0,destination.r),blend(1,destination.g),blend(2,destination.b),blend(3,destination.a)};++painted;
        }
    }return painted;
}
inline size_t compositeLayoutWindows(TextureImage& target,const NativeLayoutResource& pose,const std::vector<TextureImage>& textures){return compositeLayoutQuads(target,pose,transformLayoutTextureCoordinates(layoutWindowQuads(pose,textures),pose),textures);}
inline void layoutCompositionTests(){
    NativeLayoutResource layout;layout.origin=1;layout.size={4,4};NativeLayoutPane pane;pane.kind="wnd1";pane.flags=1;pane.scale={1,1};pane.size={4,4};pane.base=4;pane.alpha=128;pane.colors.fill(0xFFFFFFFF);pane.uv={{0,0,1,0,0,1,1,1}};pane.windowFrames={{0,0}};layout.panes={pane};NativeLayoutMaterial material;material.resourceBits=0x111;material.textures={{0,0,0}};material.textureSrt={{0,0,0,1,1}};material.coordinateGenerators={{1,4,30}};material.tevColors[1]={0,0,0,255};layout.materials={material};TextureImage texture{1,1,6,{{255,255,255,255}}},frame{4,4,6,std::vector<Pixel>(16,{200,100,50,255})};assetCheck(compositeLayoutWindows(frame,layout,{texture})==16,"Layout window seams/gaps/overdraw mismatch");for(auto p:frame.pixels)assetCheck(p.r==100&&p.g==50&&p.b==25,"Layout source-alpha blend mismatch");layout.panes[0].alpha=0;auto prior=frame.pixels;assetCheck(compositeLayoutWindows(frame,layout,{texture})==0,"Invisible layout composed pixels");for(size_t i=0;i<prior.size();++i)assetCheck(frame.pixels[i].r==prior[i].r&&frame.pixels[i].a==prior[i].a,"Zero alpha modified target");
}


