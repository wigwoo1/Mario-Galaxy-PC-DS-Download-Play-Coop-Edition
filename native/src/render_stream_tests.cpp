#include "native_render_stream.hpp"
#include <cassert>
#include <cstdio>
int main(){
 Native::GpuResources resources;std::vector<u8> bytes;
 auto word=[&](u32 v){for(int shift=24;shift>=0;shift-=8)bytes.push_back(v>>shift);};
 auto cp=[&](u8 reg,u32 value){bytes.insert(bytes.end(),{8,reg});word(value);};
 auto bp=[&](unsigned reg,unsigned value){bytes.push_back(0x61);word((reg<<24)|value);};
 auto fl=[&](float v){u32 bits;std::memcpy(&bits,&v,4);word(bits);};
 cp(0x50,1<<9);cp(0x70,1|(4<<1));
 bytes.push_back(0x10);word(11u<<16);for(float v:{1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f,0.f})fl(v);
 auto triangle=[&](){bytes.insert(bytes.end(),{0x90,0,3});for(float v:{0.f,0.f,-.5f,1.f,0.f,-.5f,0.f,1.f,-.5f})fl(v);};
 bp(0x41,24);triangle();bp(0x48,7);bp(0x47,7);bp(0x41,16);triangle();bp(0x52,0);bp(0x45,2);
 NativeRenderStream stream;stream.decode(bytes.data(),bytes.size(),resources);
 const auto& ops=stream.operations();assert(ops.size()==5);
 assert(ops[0].kind==NativeRenderOperation::Kind::Draw&&ops[0].state.bp(0x41)==24&&ops[0].triangles.size()==1);
 assert(ops[1].kind==NativeRenderOperation::Kind::Token&&ops[1].token==7);
 assert(ops[2].kind==NativeRenderOperation::Kind::Draw&&ops[2].state.bp(0x41)==16);
 assert(ops[3].kind==NativeRenderOperation::Kind::Copy&&ops[4].kind==NativeRenderOperation::Kind::Finish);
 auto malformed=bytes;malformed.push_back(0x61);bool rejected=false;
 try{stream.decode(malformed.data(),malformed.size(),resources);}catch(const std::length_error&){rejected=true;}assert(rejected&&stream.operations().size()==5&&stream.state().bp(0x41)==16);
 auto address=resources.add(bytes.data(),bytes.size());std::vector<u8> nested{0x40};
 for(int shift=24;shift>=0;shift-=8)nested.push_back(address>>shift);for(int shift=24;shift>=0;shift-=8)nested.push_back(bytes.size()>>shift);
 stream.decode(nested.data(),nested.size(),resources);assert(stream.operations().size()==5&&stream.operations()[1].token==7);
 std::puts("PASS mixed GX register/draw/token/copy/finish order, per-draw state, nested lists and atomic rejection");
}
