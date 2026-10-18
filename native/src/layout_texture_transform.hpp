#pragma once
#include "layout_window_mesh.hpp"
// Native adaptation of nw4r centered texture-SRT matrices; single UV generator supported.
inline std::array<float,2> layoutMaterialUv(const NativeLayoutMaterial& material,std::array<float,2> uv){
    assetCheck(material.coordinateGenerators.size()==1,"Unsupported native layout coordinate generator count");const auto& generator=material.coordinateGenerators.front();assetCheck(generator[0]==1&&generator[1]==4,"Unsupported native layout texture coordinate source/type");if(generator[2]==60)return uv;
    assetCheck(generator[2]>=30&&generator[2]<=57&&(generator[2]-30)%3==0,"Invalid layout texture matrix selector");size_t index=(generator[2]-30)/3;assetCheck(index<material.textureSrt.size(),"Missing layout texture matrix");const auto& srt=material.textureSrt[index];float angle=srt[2]*0.017453292519943295f,c=std::cos(angle),s=std::sin(angle),x=(uv[0]-.5f)*srt[3],y=(uv[1]-.5f)*srt[4];std::array<float,2> result{c*x-s*y+.5f+srt[0],s*x+c*y+.5f+srt[1]};for(float coordinate:result)assetCheck(std::isfinite(coordinate),"Layout texture coordinate overflow");return result;
}
inline std::vector<NativeLayoutQuad> transformLayoutTextureCoordinates(std::vector<NativeLayoutQuad> mesh,const NativeLayoutResource& layout){
    for(auto& quad:mesh){assetCheck(quad.material<layout.materials.size(),"Missing layout quad material");for(unsigned corner=0;corner<4;++corner){auto uv=layoutMaterialUv(layout.materials[quad.material],{quad.uv[corner*2],quad.uv[corner*2+1]});quad.uv[corner*2]=uv[0];quad.uv[corner*2+1]=uv[1];}}return mesh;
}
inline void layoutTextureTransformTests(){
    NativeLayoutMaterial material;material.coordinateGenerators={{1,4,30}};material.textureSrt={{0,0,90,1,1}};auto uv=layoutMaterialUv(material,{0,0});assetCheck(std::abs(uv[0]-1)<.0001f&&std::abs(uv[1])<.0001f,"Centered layout UV rotation mismatch");material.textureSrt={{.5f,0,0,2,1}};assetCheck(layoutMaterialUv(material,{0,0})==std::array<float,2>{0,0}&&layoutMaterialUv(material,{1,1})==std::array<float,2>{2,1},"Layout texture translation/scale mismatch");material.coordinateGenerators[0][2]=60;assetCheck(layoutMaterialUv(material,{.2f,.7f})==std::array<float,2>{.2f,.7f},"Identity layout UV generator mismatch");
}
