#include "native_d3d_rgb565_copy.hpp"
#include "native_d3d_command_submitter.hpp"
#include <revolution/gd/GDBase.h>
#include "native_d3d_render_stream.hpp"
#include <revolution/gx/GXCpu2Efb.h>
#include <revolution/gx/GXManage.h>
#include <cassert>
#include <cstdio>
static unsigned callbacks,finishes,faultCalls;
static void faultCallback(u16){++faultCalls;throw std::runtime_error("Injected callback execution failure");}
static void finishCallback(){++finishes;}
static void tokenCallback(u16 token){assert(token==7);u32 depth=0;GXPeekZ(32,32,&depth);assert(depth>0x7ffff0&&depth<0x800010);++callbacks;}
int main(){
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 Native::GpuResources resources;std::vector<u8> capture(512,0xa5);auto captureAddress=resources.addWritable(capture.data(),capture.size());
 std::vector<u8> bytes;auto word=[&](u32 v){for(int shift=24;shift>=0;shift-=8)bytes.push_back(v>>shift);};
 auto fl=[&](float v){u32 bits;std::memcpy(&bits,&v,4);word(bits);};
 auto cp=[&](u8 reg,u32 value){bytes.insert(bytes.end(),{8,reg});word(value);};
 auto bp=[&](unsigned reg,unsigned value){bytes.push_back(0x61);word((reg<<24)|value);};
 auto xf=[&](unsigned address,std::initializer_list<float> values){bytes.push_back(0x10);word(((values.size()-1)<<16)|address);for(float v:values)fl(v);};
 auto xword=[&](unsigned address,u32 value){bytes.push_back(0x10);word(address);word(value);};
 cp(0x50,(1<<9)|(1<<13));cp(0x70,1|(4<<1)|(5<<14));
 xf(0,{1,0,0,0,0,1,0,0,0,0,1,0});
 xf(0x1020,{1,0,1,0,1,0});xword(0x1026,1);
 xf(0x101a,{32,-32,16777216.f,374,374,16777216.f});
 xword(0x100e,1);xword(0x1010,1);
 bp(0x20,(342<<12)|342);bp(0x21,(405<<12)|405);bp(0x59,171|(171<<10));
 bp(0x41,24);bp(0x40,23);bp(0xf3,(7<<16)|(7<<19));bp(0xf6,4);bp(0xf7,14);
 bp(0xc0,(15<<12)|(10<<8)|(12<<4)|15|(1<<19));bp(0xc1,(7<<13)|(7<<10)|(7<<7)|(5<<4)|(1<<19));
 auto triangle=[&](float z,u8 r,u8 b){bytes.insert(bytes.end(),{0x90,0,3});for(auto xy:std::array<std::array<float,2>,3>{{{-.8f,-.8f},{0,.8f},{.8f,-.8f}}}){fl(xy[0]);fl(xy[1]);fl(z);bytes.insert(bytes.end(),{r,0,b,255});}};
 triangle(-.5f,255,0);bp(0x49,24|(24<<10));bp(0x4a,15|(15<<10));bp(0x4b,captureAddress>>5);bp(0x4d,4);bp(0x52,4<<4);bp(0x48,7);bp(0x47,7);triangle(-.8f,0,255);bp(0x45,2);
 NativeRenderStream stream;stream.decode(bytes.data(),bytes.size(),resources);
 Native::D3dEfb efb(*context.Get(),64,64);float clear[]={0,0,0,1};efb.clear(clear,1);
 NativeD3dRenderStream renderer(*context.Get());NativeD3dRgb565Copy copyPipeline(efb,resources,*context.Get());
 auto missingCopy=stream.operations();missingCopy.push_back(NativeRenderOperation{NativeRenderOperation::Kind::Copy,stream.state()});
 bool rejected=false;try{renderer.execute(missingCopy,efb,resources);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 auto before=efb.readColor();assert(before[(32*64+32)*4]==0&&before[(32*64+32)*4+2]==0&&callbacks==0);
 GXSetDrawDoneCallback(finishCallback);GXSetDrawSyncCallback(tokenCallback);
 renderer.execute(stream.operations(),efb,resources,copyPipeline);GXSetDrawSyncCallback(nullptr);GXSetDrawDoneCallback(nullptr);
 auto color=efb.readColor();assert(callbacks==1&&finishes==1&&color[(32*64+32)*4]==0&&color[(32*64+32)*4+2]==255);
  efb.clear(clear,1);callbacks=0;finishes=0;
 NativeD3dCommandSubmitter submitter(*context.Get(),efb,resources,copyPipeline);
 std::vector<u8> storage(bytes.size()+128);GDLObj writer;GDInitGDLObj(&writer,storage.data(),u32(storage.size()));GDSetCurrent(&writer);
 {
  Native::CommandSubmissionBinding binding(submitter);GXSetDrawSyncCallback(tokenCallback);GXSetDrawDoneCallback(finishCallback);
  std::memcpy(writer.ptr,bytes.data(),bytes.size());writer.ptr+=bytes.size();
  GXFlush();assert(GDGetCurrOffset()==0);GXDrawDone();submitter.waitForFinish();
  GDWrite_u8(0xff);bool malformedRejected=false;try{GXFlush();}catch(const std::invalid_argument&){malformedRejected=true;}assert(malformedRejected&&GDGetCurrOffset()==1);
  writer.ptr=writer.start;GXSetDrawSyncCallback(faultCallback);GXSetDrawSync(9);assert(GDGetCurrOffset()==0&&faultCalls==1);
  bool deferred=false;try{submitter.waitForFinish();}catch(const std::runtime_error&){deferred=true;}assert(deferred&&faultCalls==1);
  GXSetDrawSyncCallback(nullptr);GXSetDrawDoneCallback(nullptr);
 }
 GDSetCurrent(nullptr);assert(callbacks==1&&finishes==2);
 color=efb.readColor();assert(color[(32*64+32)*4+2]==255);
  auto captured=decodeGxTexture(BinaryView(capture),16,16,4,0,BinaryView(capture,0,0));
 auto pixel=captured.pixels[8*16+8];assert(pixel.r==255&&pixel.g==0&&pixel.b==0);
 assert(std::memcmp(capture.data(),resources.resolve(captureAddress,512),512)==0);
 std::puts("PASS mixed GX stream draws on actual D3D GPU with fenced depth callback between occluding triangles");
}






