#include <Native/DisplayCopyBinding.hpp>
#include <Native/GxCommandState.hpp>
#include <Native/GpuResources.hpp>
#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
void emitNativeDisplayCopy(void*,GXBool,const Native::GxCommandState&,Native::GpuResources&);
int main(){
 Native::GpuResources resources;alignas(32) u8 framebuffer[256]{},commands[128]{};
 resources.addWritable(framebuffer,sizeof(framebuffer));Native::GxCommandState state;
 GDLObj dl;GDInitGDLObj(&dl,commands,sizeof(commands));GDSetCurrent(&dl);
 GXSetDispCopySrc(17,23,16,4);GXSetDispCopyDst(16,4);
 bool unbound=false;try{GXCopyDisp(framebuffer,GX_FALSE);}catch(const std::logic_error&){unbound=true;}assert(unbound);
 Native::DisplayCopyBinding binding(state,resources);GXCopyDisp(framebuffer,GX_FALSE);assert(GDGetCurrOffset()==25);
 GXPixModeSync();assert(GDGetCurrOffset()==30&&commands[25]==0x61&&commands[26]==0x43&&commands[27]==0&&commands[28]==0&&commands[29]==0);
 Native::GxCommandState decoded;decoded.consume(commands,25,&resources);assert(decoded.pendingCopies().size()==1);
 auto copy=decoded.pendingCopies()[0];assert(copy.display()&&!copy.clear()&&copy.left()==17&&copy.top()==23&&copy.width()==16&&copy.height()==4&&copy.strideBytes()==32&&copy.destinationAddress()==resources.address(framebuffer));
 GDInitGDLObj(&dl,commands,sizeof(commands));GDWriteBPCmd(0x40000012);GDWriteBPCmd(0x41000003);GDWriteBPCmd(0x43000040);
 GXCopyDisp(framebuffer,GX_TRUE);assert(GDGetCurrOffset()==70);Native::GxCommandState cleared;cleared.consume(commands,70,&resources);
 assert(cleared.bp(0x40)==0x12&&cleared.bp(0x41)==3&&cleared.bp(0x43)==0x40&&cleared.pendingCopies().back().clear());
 for(auto&v:commands)v=0xa5;GDInitGDLObj(&dl,commands,54);
 bool rejected=false;try{GXCopyDisp(framebuffer,GX_TRUE);}catch(const std::length_error&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);for(auto v:commands)assert(v==0xa5);
 // The observer also scans normal draw lists. It must skip each primitive's
 // vertex payload so display-copy commands later in the list remain visible.
 std::vector<u8> drawStream;auto cp=[&](u8 reg,u32 value){drawStream.insert(drawStream.end(),{8,reg,u8(value>>24),u8(value>>16),u8(value>>8),u8(value)});};
 auto word=[&](u32 value){drawStream.insert(drawStream.end(),{u8(value>>24),u8(value>>16),u8(value>>8),u8(value)});};
 cp(0x50,1u<<9);cp(0x70,1u|(4u<<1));
 drawStream.insert(drawStream.end(),{0x80,0,4});drawStream.resize(drawStream.size()+4*12,0);
 drawStream.insert(drawStream.end(),{0xb8,0,2});drawStream.resize(drawStream.size()+2*12,0);
 auto truncatedDrawStream=drawStream;truncatedDrawStream.pop_back();
 drawStream.push_back(0x61);word(0x52000000);
 Native::GxCommandState drawState;drawState.consume(drawStream.data(),drawStream.size());
 assert(drawState.pendingCopies().size()==1&&drawState.vertexPayloadSize(0)==12);
 Native::GxCommandState malformed;bool truncatedDraw=false;try{malformed.consume(truncatedDrawStream.data(),truncatedDrawStream.size());}catch(const std::length_error&){truncatedDraw=true;}assert(truncatedDraw&&malformed.pendingCopies().empty());
 GDSetCurrent(nullptr);resources.remove(framebuffer);
 std::puts("PASS display-copy command observer: live state mirroring, bounded primitive skipping, later copy detection and truncation rejection");
}

