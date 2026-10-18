#pragma once
#include <Native/D3dModelDraw.hpp>
namespace Native {
inline void clearD3dEfbRegion(ID3D11DeviceContext& context,D3dEfb& efb,unsigned left,unsigned top,unsigned width,unsigned height,const float rgba[4],float depth){
 if(!width||!height||left>efb.getWidth()||top>efb.getHeight()||width>efb.getWidth()-left||height>efb.getHeight()-top||!rgba||!std::isfinite(depth)||depth<0||depth>1)throw std::invalid_argument("Invalid EFB region clear");
 for(unsigned i=0;i<4;++i)if(!std::isfinite(rgba[i]))throw std::invalid_argument("Invalid EFB region clear color");
 GxCommandState state;
 auto bp=[&](unsigned reg,unsigned value){unsigned word=(reg<<24)|value;unsigned char b[]={0x61,(unsigned char)(word>>24),(unsigned char)(word>>16),(unsigned char)(word>>8),(unsigned char)word};state.consume(b,5);};
 bp(0x40,31);bp(0x41,24);
 D3dModelDraw draw(context);draw.setDepthMode(state);draw.setBlendMode(state);draw.setNativeScissor(left,top,left+width,top+height);
 std::vector<ProjectedVertex> vertices(6);const float xy[6][2]={{-1,-1},{-1,1},{1,1},{-1,-1},{1,1},{1,-1}};
 for(unsigned i=0;i<6;++i){vertices[i].position[0]=xy[i][0];vertices[i].position[1]=xy[i][1];vertices[i].position[2]=depth-1;vertices[i].position[3]=1;std::copy(rgba,rgba+4,vertices[i].color);}
 draw.draw(efb,vertices);
}
}
