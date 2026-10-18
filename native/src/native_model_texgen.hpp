#pragma once
#include "native_shape_vertices.hpp"
// Regular texgen only; source attributes and matrix selectors remain authored.
inline std::array<float,3> generateModelTexCoord(const Native::GxCommandState& state,
                                                const NativeShapeVertex& vertex,unsigned slot){
 if(slot>=8)throw std::out_of_range("GX texgen slot");
 if(!state.xfLoaded(0x1040+slot))throw std::runtime_error("Texgen descriptor not loaded");
 unsigned config=state.xf(0x1040+slot),type=(config>>4)&7,row=(config>>7)&31;
 if(type!=0)throw std::invalid_argument("Unsupported nonregular model texgen");
 std::array<float,3> input{};
 if(row==0)input=vertex.objectPosition;
 else if(row==1){if(!vertex.hasNormal)throw std::invalid_argument("Texgen normal missing");input=vertex.normal;}
 else if(row>=5&&row<=12){unsigned channel=row-5;if(vertex.hasTexCoords[channel])input=vertex.texCoords[channel];else if(channel==0&&vertex.hasTexCoord0)input=vertex.texCoord0;else throw std::invalid_argument("Texgen texture coordinate missing");}
 else throw std::invalid_argument("Unsupported texgen source row");
 // AB11 input replaces the third component before the matrix multiply.
 if(!(config&4))input[2]=1;
 auto uv=state.transformTexture(vertex.textureMatrices[slot],input,(config&2)!=0);
 if(state.xf(0x1012)&1){
  if(!state.xfLoaded(0x1050+slot))throw std::runtime_error("Dual texgen descriptor not loaded");
  unsigned dual=state.xf(0x1050+slot);
  if(dual&256){
   float length=std::sqrt(uv[0]*uv[0]+uv[1]*uv[1]+uv[2]*uv[2]);
   if(!std::isfinite(length)||length==0)throw std::invalid_argument("Invalid normalized dual texgen");
   for(float& v:uv)v/=length;
  }
  uv=state.transformTexture(dual&63,uv,true,true);
 }
 return uv;
}


