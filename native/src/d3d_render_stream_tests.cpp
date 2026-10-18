#include "native_d3d_command_submitter.hpp"
#include <revolution/gd/GDBase.h>
#include "native_d3d_render_stream.hpp"
#include <revolution/gx/GXCpu2Efb.h>
#include <revolution/gx/GXManage.h>
#include <cassert>
#include <cstdio>
#include <condition_variable>
#include <mutex>
#include <thread>
extern "C" void GXWaitDrawDone();
static unsigned callbacks,finishes,faultCalls;
static void faultCallback(u16){++faultCalls;throw std::runtime_error("Injected callback execution failure");}
static void finishCallback(){++finishes;}
static void tokenCallback(u16 token){assert(token==7);u32 depth=0;GXPeekZ(32,32,&depth);assert(depth>0x7ffff0&&depth<0x800010);++callbacks;}
int main(){
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
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
 triangle(-.5f,255,0);bp(0x48,7);bp(0x47,7);triangle(-.8f,0,255);bp(0x45,2);
 Native::GpuResources resources;NativeRenderStream stream;stream.decode(bytes.data(),bytes.size(),resources);
 Native::D3dEfb efb(*context.Get(),64,64);float clear[]={0,0,0,1};efb.clear(clear,1);
 NativeD3dRenderStream renderer(*context.Get());
 auto missingCopy=stream.operations();missingCopy.push_back(NativeRenderOperation{NativeRenderOperation::Kind::Copy,stream.state()});
 bool rejected=false;try{renderer.execute(missingCopy,efb,resources);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 auto before=efb.readColor();assert(before[(32*64+32)*4]==0&&before[(32*64+32)*4+2]==0&&callbacks==0);
 GXSetDrawDoneCallback(finishCallback);GXSetDrawSyncCallback(tokenCallback);
 renderer.execute(stream.operations(),efb,resources);GXSetDrawSyncCallback(nullptr);GXSetDrawDoneCallback(nullptr);
 auto color=efb.readColor();assert(callbacks==1&&finishes==1&&color[(32*64+32)*4]==0&&color[(32*64+32)*4+2]==255);
  efb.clear(clear,1);callbacks=0;finishes=0;
 NativeD3dCommandSubmitter submitter(*context.Get(),efb,resources);
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
 {
  std::mutex gateMutex;std::condition_variable gateChanged;bool copyEntered=false,releaseCopy=false,abortSubmitted=false,alarmFinished=false;
  auto slowCopy=[&](const Native::GxCommandState::CopyOperation&){std::unique_lock<std::mutex> lock(gateMutex);copyEntered=true;gateChanged.notify_all();gateChanged.wait(lock,[&]{return releaseCopy;});};
  NativeD3dCommandSubmitter abortable(*context.Get(),efb,resources,slowCopy);
  std::vector<u8> abortStorage(256);GDLObj abortWriter;GDInitGDLObj(&abortWriter,abortStorage.data(),u32(abortStorage.size()));GDSetCurrent(&abortWriter);
  Native::CommandSubmissionBinding abortBinding(abortable);
  const u8 abortStream[]={0x61,0x52,0,0,0,0x61,0x48,0,0,7,0x61,0x47,0,0,7};
  std::memcpy(abortWriter.ptr,abortStream,sizeof(abortStream));abortWriter.ptr+=sizeof(abortStream);
  std::thread watchdog([&]{
   GDSetCurrent(&abortWriter); // Writers are thread-local; explicitly attach this serialized fixture.
   {std::unique_lock<std::mutex> lock(gateMutex);gateChanged.wait(lock,[&]{return copyEntered;});}
   GXAbortFrame();Native::appendGxAbortResetCommand();
   {std::lock_guard<std::mutex> lock(gateMutex);abortSubmitted=true;}gateChanged.notify_all();
   GXSetDrawDone();Native::markGxAbortRecoveryReady();
   {std::lock_guard<std::mutex> lock(gateMutex);alarmFinished=true;}gateChanged.notify_all();
  });
  std::thread renderThread([&]{GDSetCurrent(&abortWriter);GXFlush();});
  {std::unique_lock<std::mutex> lock(gateMutex);gateChanged.wait(lock,[&]{return abortSubmitted;});releaseCopy=true;}gateChanged.notify_all();
  renderThread.join();GXWaitDrawDone();watchdog.join();assert(alarmFinished);
 }
 GDSetCurrent(nullptr);
 {
  NativeRenderOperation op{NativeRenderOperation::Kind::Draw,stream.state()};
  auto setBp=[&](unsigned reg,u32 value){u32 word=(reg<<24)|value;u8 command[]={0x61,u8(word>>24),u8(word>>16),u8(word>>8),u8(word)};op.state.consume(command,5);};
  auto setXf=[&](unsigned reg,u32 value){u8 command[]={0x10,0,0,u8(reg>>8),u8(reg),u8(value>>24),u8(value>>16),u8(value>>8),u8(value)};op.state.consume(command,9);};
  std::vector<u8> target(64),displacement(64);
  for(unsigned i=0;i<16;++i){target[i*2]=255;target[i*2+1]=10+50*(i%4);target[32+i*2]=20+50*(i%4);target[33+i*2]=30+50*(i%4);displacement[i*2]=(i%4)?128:129;displacement[i*2+1]=213;displacement[32+i*2]=displacement[33+i*2]=128;}
  auto ta=resources.add(target.data(),target.size()),ia=resources.addWritable(displacement.data(),displacement.size());
  setBp(0,(1<<16)|2);setBp(0x28,64);setBp(0x27,1|(1<<3));setBp(0x25,1);
  setBp(0x30,3);setBp(0x31,0);setBp(0x32,1);setBp(0x33,0);
  setBp(6,512|(2<<22));setBp(7,0);setBp(8,1<<22);setBp(0x10,(7<<4)|(1<<9));
  setBp(0x80,0);setBp(0x81,0);setBp(0x84,0);setBp(0x85,0);
  setBp(0x88,3|(6<<20));setBp(0x89,1|(6<<20));setBp(0x94,ta>>5);setBp(0x95,ia>>5);
  setBp(0xc0,(15<<12)|(8<<8)|(12<<4)|15|(1<<19));setBp(0xc1,(7<<13)|(7<<10)|(7<<7)|(4<<4)|(1<<19));
  setXf(0x1012,0);setXf(0x1040,5<<7);setXf(0x1041,6<<7);
  auto input=std::array<std::array<float,3>,3>{{{-.8f,-.8f,-.5f},{0,.8f,-.5f},{.8f,-.8f,-.5f}}};
  for(auto position:input){NativeShapeVertex vertex{};vertex.position=vertex.objectPosition=position;vertex.textureMatrices[0]=vertex.textureMatrices[1]=60;vertex.hasTexCoords[0]=vertex.hasTexCoords[1]=true;vertex.texCoords[0]={.125f,.5f,1};vertex.texCoords[1]={.75f,.5f,1};op.vertices.push_back(vertex);}
  op.triangles={{0,1,2}};
  auto batch=renderer.prepare({op},resources);
  assert(batch.prepared[0].vertices[0].extraUv[0][0]==.75f);
  efb.clear(clear,1);renderer.executePrepared(batch,efb);auto pixel=efb.readColor();unsigned center=(32*64+32)*4;
  assert(pixel[center]==60&&pixel[center+1]==70&&pixel[center+2]==80);
  // A capture/resource edit between prepare and draw must refresh the indirect map too.
  for(unsigned i=0;i<16;++i)displacement[i*2]=128;
  resources.write(ia,displacement.data(),displacement.size());
  efb.clear(clear,1);renderer.executePrepared(batch,efb);pixel=efb.readColor();assert(pixel[center]==10&&pixel[center+1]==20&&pixel[center+2]==30);
  resources.remove(target.data());resources.remove(displacement.data());
  std::puts("PASS indirect stream integration: separate texture binding/texgen, coordinate scale and prepared-resource refresh with exact GPU pixel readback");
 }
 std::puts("PASS mixed GX stream draws on actual D3D GPU with fenced depth callback between occluding triangles");
}




