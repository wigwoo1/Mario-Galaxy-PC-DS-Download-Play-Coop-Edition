#include <revolution/gx/GXTransform.h>
#include <revolution/gd/GDBase.h>
#include <array>
#include <stdexcept>
namespace {std::array<u32,4> retained{};bool loaded=false;}
extern "C" void GXSetScissor(u32 left,u32 top,u32 width,u32 height){
 if(!width||!height||left>1705||top>1705||width>1706-left||height>1706-top)throw std::invalid_argument("GX scissor bounds");
 const u32 l=left+342,t=top+342,r=l+width-1,b=t+height-1;
 GDOverflowCheck(10);GDWriteBPCmd(0x20000000|t|(l<<12));GDWriteBPCmd(0x21000000|b|(r<<12));
 retained={left,top,width,height};loaded=true;
}
extern "C" void GXGetScissor(u32* left,u32* top,u32* width,u32* height){
 if(!left||!top||!width||!height||!loaded)throw std::invalid_argument("GX scissor destination/state");
 *left=retained[0];*top=retained[1];*width=retained[2];*height=retained[3];
}
extern "C" void GXSetScissorBoxOffset(s32 x,s32 y){
 if(x< -342||x>1705||y< -342||y>1705)throw std::invalid_argument("GX scissor offset bounds");
 GDOverflowCheck(5);GDWriteBPCmd(0x59000000|u32(x+342)/2|(u32(y+342)/2<<10));
}
