#include "native_alpha8_copy.hpp"
#include <cassert>
#include <cstdio>
int main(){
 TextureImage source{17,11,6,std::vector<Pixel>(187)};
 for(unsigned y=0;y<11;++y)for(unsigned x=0;x<17;++x)source.pixels[y*17+x]={u8(x*13),u8(y*17),u8((x+y)*7),u8((x+y*17)*19)};
 std::vector<u8> memory(256,0xa5);Native::GpuResources resources;auto address=resources.addWritable(memory.data(),memory.size());
 Native::GxCommandState::CopyOperation copy{};copy.registers[0x49]=2|(3<<10);copy.registers[0x4a]=8|(4<<10);copy.registers[0x4b]=address>>5;copy.registers[0x4d]=3;copy.registers[0x43]=1;copy.registers[0x52]=0x10073;
 writeAlpha8Copy(source,copy,resources);
 for(unsigned y=0;y<5;++y)for(unsigned x=0;x<9;++x){u8 a=source.pixels[(y+3)*17+x+2].a;assert(memory[(y/4)*96+(x/8)*32+(y%4)*8+x%8]==u8((a&252)|(a>>6)));}
 for(unsigned i=64;i<96;++i)assert(memory[i]==0xa5);assert(memory[159]==0xa5&&memory[255]==0xa5);
 std::vector<u8> compact(128);std::memcpy(compact.data(),memory.data(),64);std::memcpy(compact.data()+64,memory.data()+96,64);
 auto decoded=decodeGxTexture(BinaryView(compact),9,5,1,0,BinaryView(compact,0,0));
 for(unsigned y=0;y<5;++y)for(unsigned x=0;x<9;++x){auto p=decoded.pixels[y*9+x];u8 a=source.pixels[(y+3)*17+x+2].a;unsigned expected=(a&252)|(a>>6);assert(p.r==expected&&p.g==expected&&p.b==expected&&p.a==expected);}
 assert(std::memcmp(memory.data(),resources.resolve(address,memory.size()),memory.size())==0);
 for(unsigned mode:{0u,2u}){copy.registers[0x43]=mode;writeAlpha8Copy(source,copy,resources);for(unsigned y=0;y<5;++y)for(unsigned x=0;x<9;++x)assert(memory[(y/4)*96+(x/8)*32+(y%4)*8+x%8]==255);}
 auto saved=memory;auto reject=[&](auto operation){bool caught=false;try{writeAlpha8Copy(source,operation,resources);}catch(const std::exception&){caught=true;}assert(caught&&memory==saved);};
 auto invalid=copy;invalid.registers[0x49]=16;reject(invalid);
 invalid=copy;invalid.registers[0x4d]=1;reject(invalid);
 invalid=copy;invalid.registers[0x4b]=(address+224)>>5;reject(invalid);
 invalid=copy;invalid.registers[0x52]|=1<<9;reject(invalid);
 invalid=copy;invalid.registers[0x43]=3;reject(invalid);
 std::vector<u8> readOnly(256,0xa5);auto readOnlyAddress=resources.add(readOnly.data(),readOnly.size());invalid=copy;invalid.registers[0x4b]=readOnlyAddress>>5;reject(invalid);for(auto value:readOnly)assert(value==0xa5);
 std::puts("PASS A8 tile encoding: offset rectangle, 8x4 tiles, padded stride, RGBA6 expansion, opaque RGB-only modes, I8 decode and synchronized snapshot");
 std::puts("PASS A8 rejection is atomic: bounds, stride, short/read-only destination, depth and unsupported scaling");
}
