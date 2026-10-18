#pragma once
#include <Native/GxCommandState.hpp>
namespace Native {
inline bool writesTextureDepth(const GxCommandState& state){
 auto z=state.depthTexture();
 if(z.operation>2||(z.operation&&z.type>2))throw std::invalid_argument("Invalid GX depth texture mode");
 // With early depth testing GX retains raster depth in the depth buffer.
 return z.operation&&!(state.bp(0x43)&64);
}
inline std::string textureDepthHlsl(const GxCommandState& state){
 auto z=state.depthTexture();int last=-1;
 if(state.bp(0)&15)for(unsigned i=0;i<state.tevStageCount();++i)if(state.tevOrder(i).enabled)last=int(i);
 std::string code="int4 zsample="+(last<0?std::string("int4(0,0,0,0)"):"textures["+std::to_string(last)+"]")+";uint zvalue=";
 code+=z.type==0?"uint(zsample.a)":z.type==1?"uint(zsample.r+zsample.a*256)":"uint(zsample.r*65536+zsample.g*256+zsample.b)";
 code+="+"+std::to_string(z.bias)+"u;";
 if(z.operation==1)code+="zvalue+=uint(clamp(o.p.z*16777216.0,0.0,16777215.0));";
 return code+"output.depth=float(zvalue&16777215u)/16777216.0;";
}
}
