#include "preview.hpp"
#include "native_rgb565_copy.hpp"
#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <new>
#include <cassert>
#include <Native/TextureCopyBinding.hpp>
#include <cstdio>
int main(){
 TextureImage source{16,16,6,std::vector<Pixel>(256)};
 for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++)source.pixels[y*16+x]={u8(x*17),u8(y*17),u8((x+y)*8),255};
 TextureRenderer renderer(source,true);auto rendered=renderer.readback();
 std::vector<u8> destination(512,0xa5);Native::GpuResources resources;auto address=resources.addWritable(destination.data(),destination.size());
 u8 commands[64]={};GDLObj list;GDInitGDLObj(&list,commands,sizeof(commands));GDSetCurrent(&list);
 // Borrowed fixture tests only the original capture method, not owned allocation.
 alignas(JUTTexture) unsigned char textureStorage[sizeof(JUTTexture)];auto* texture=new(textureStorage) JUTTexture;
 ResTIMG info{};info.mWidth=16;info.mHeight=16;texture->mTIMG=&info;texture->mImage=destination.data();texture->setCaptureFlag(true);
 Native::GxCommandState state;{Native::TextureCopyBinding binding(state,resources);texture->capture(0,0,GX_TF_RGB565,false,0);}
 assert(GDGetCurrOffset()==30);state.consume(commands,GDGetCurrOffset());assert(state.pendingCopies().size()==1);
 writeRgb565Copy(rendered,state.pendingCopies()[0],resources);
 auto decoded=decodeGxTexture(BinaryView(destination),16,16,4,0,BinaryView(destination,0,0));
 for(unsigned i=0;i<256;i++){auto p=rendered.pixels[i];auto expected=rgb565(u16(p.r>>3)<<11|u16(p.g>>2)<<5|p.b>>3);assert(decoded.pixels[i].r==expected.r&&decoded.pixels[i].g==expected.g&&decoded.pixels[i].b==expected.b);}
 assert(std::memcmp(destination.data(),resources.resolve(address,512),512)==0);
 state.clearPendingCopies();GDInitGDLObj(&list,commands,sizeof(commands));
 GDWriteBPCmd(0x40000002);GDWriteBPCmd(0x41000003);GDWriteBPCmd(0x43000040);state.consume(commands,GDGetCurrOffset());
 GDInitGDLObj(&list,commands,sizeof(commands));{Native::TextureCopyBinding binding(state,resources);GXCopyTex(destination.data(),1);}assert(GDGetCurrOffset()==55);
 state.consume(commands,GDGetCurrOffset());assert(state.pendingCopies().size()==1);const auto& operation=state.pendingCopies()[0];assert(operation.clear()&&operation.registers[0x40]==15&&operation.registers[0x41]==0&&operation.registers[0x43]==0);assert(state.bp(0x40)==2&&state.bp(0x41)==3&&state.bp(0x43)==64);
 GDInitGDLObj(&list,commands,sizeof(commands));bool unbound=false;try{GXCopyTex(destination.data(),0);GXPixModeSync();}catch(const std::logic_error&){unbound=true;}assert(unbound&&GDGetCurrOffset()==0);
 {Native::TextureCopyBinding outer(state,resources);Native::GpuResources empty;bool missing=false;{Native::TextureCopyBinding inner(state,empty);try{GXCopyTex(destination.data(),0);GXPixModeSync();}catch(const std::invalid_argument&){missing=true;}}assert(missing&&GDGetCurrOffset()==0);GXCopyTex(destination.data(),0);assert(GDGetCurrOffset()==25);}
 GDInitGDLObj(&list,commands,sizeof(commands));{Native::TextureCopyBinding binding(state,resources);GXPixModeSync();}assert(GDGetCurrOffset()==5&&commands[0]==0x61&&commands[1]==0x43&&commands[2]==0&&commands[3]==0&&commands[4]==64);
 info.mWidth=4;info.mHeight=4;state.clearPendingCopies();GDInitGDLObj(&list,commands,sizeof(commands));{Native::TextureCopyBinding binding(state,resources);texture->capture(1,2,GX_TF_RGB565,true,0);}state.consume(commands,GDGetCurrOffset());assert(state.pendingCopies().size()==1);const auto& mip=state.pendingCopies()[0];assert(mip.left()==1&&mip.top()==2&&mip.width()==8&&mip.height()==8&&(mip.registers[0x52]&(1u<<9)));
 texture->setCaptureFlag(false);GDInitGDLObj(&list,commands,sizeof(commands));texture->capture(0,0,GX_TF_RGB565,false,0);assert(GDGetCurrOffset()==0);
 GDSetCurrent(nullptr);
 std::puts("PASS Direct3D WARP readback -> decoded copy operation -> writable RGB565 texture roundtrip");
}








