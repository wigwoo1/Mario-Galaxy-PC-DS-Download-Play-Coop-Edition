#pragma once
#include <Native/D3dModelDraw.hpp>
#include "native_shape_vertices.hpp"
#include "native_model_texgen.hpp"
#include <JSystem/JGeometry.hpp>
#include <Native/DiffuseLighting.hpp>
#include <revolution/gx/GXEnum.h>
inline bool gxTevConsumesRasterColor(const Native::GxCommandState& state){
 for(unsigned stage=0;stage<state.tevStageCount();++stage){
  const auto tev=state.tevStage(stage);
  if(std::find(tev.color.inputs.begin(),tev.color.inputs.end(),GX_CC_RASC)!=tev.color.inputs.end()||
     std::find(tev.color.inputs.begin(),tev.color.inputs.end(),GX_CC_RASA)!=tev.color.inputs.end()||
     std::find(tev.alpha.inputs.begin(),tev.alpha.inputs.end(),GX_CA_RASA)!=tev.alpha.inputs.end())return true;
 }
 return false;
}
// View-space original model vertices -> GX clip-space triangles. Material color is supplied by caller.
inline std::vector<Native::ProjectedVertex> projectModelTriangles(
 const std::vector<NativeShapeVertex>& vertices,const std::vector<std::array<u32,3>>& triangles,
 const TProj3f& projection,const std::array<float,4>& color,const Native::GxCommandState* material=nullptr,bool lighting=false){
 if(lighting&&!material)throw std::invalid_argument("Lighting requires authored GX state");
 if(triangles.size()>UINT32_MAX/(3*sizeof(Native::ProjectedVertex)))throw std::length_error("Model triangle batch too large");
 for(const auto& row:projection.mMtx)for(float value:row)if(!std::isfinite(value))throw std::invalid_argument("Nonfinite model projection");
 for(float value:color)if(!std::isfinite(value))throw std::invalid_argument("Nonfinite model color");
 std::vector<Native::ProjectedVertex> output;output.reserve(triangles.size()*3);
 for(const auto& triangle:triangles)for(u32 index:triangle){
  if(index>=vertices.size())throw std::out_of_range("Model triangle vertex index");
  const auto& position=vertices[index].position;for(float value:position)if(!std::isfinite(value))throw std::invalid_argument("Nonfinite model vertex");
  Native::ProjectedVertex vertex{};
  for(unsigned row=0;row<4;++row)vertex.position[row]=projection.mMtx[row][0]*position[0]+projection.mMtx[row][1]*position[1]+projection.mMtx[row][2]*position[2]+projection.mMtx[row][3];
  std::copy(color.begin(),color.end(),vertex.color);
  if(lighting){
   for(bool alpha:{false,true}){
    auto control=material->lightingChannel(0,alpha);
    if((control.materialVertex||(control.enabled&&control.ambientVertex))&&!vertices[index].hasColor[0])throw std::invalid_argument("Model vertex color missing for lighting at vertex "+std::to_string(index));
    if(control.enabled&&control.lightMask&&(control.diffuse||control.attenuation)&&!vertices[index].hasNormal)throw std::invalid_argument("Model lighting normal missing");
   }
   auto lit=Native::evaluateDiffuseLighting(*material,0,position,vertices[index].transformedNormal,vertices[index].colors[0]);
   for(unsigned component=0;component<4;++component)vertex.color[component]=lit[component]/255.f;
  }
  if(material){
   const auto used=Native::materialTexCoordSlots(*material);
   for(unsigned slot=0;slot<8;++slot)if(used[slot]){
    auto uv=generateModelTexCoord(*material,vertices[index],slot);
    std::copy(uv.begin(),uv.end(),slot?vertex.extraUv[slot-1]:vertex.uv);
   }
  }
  output.push_back(vertex);
 }
 return output;
}



// Runtime overload consumes the camera projection actually emitted into GX XF registers.
inline std::vector<Native::ProjectedVertex> projectModelTriangles(
 const std::vector<NativeShapeVertex>& vertices,const std::vector<std::array<u32,3>>& triangles,
 const Native::GxCommandState& state,const std::array<float,4>& color,bool lighting=false){
 auto matrix=state.projectionMatrix();TProj3f projection;
 for(unsigned row=0;row<4;++row)for(unsigned col=0;col<4;++col)projection.mMtx[row][col]=matrix[row][col];
 return projectModelTriangles(vertices,triangles,projection,color,&state,lighting);
}
