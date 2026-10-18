#include "native_rgb565_copy.hpp"
#include <cassert>
#include <cstdio>
int main(){
 TextureImage image{9,9,6,std::vector<Pixel>(81)};for(unsigned y=0;y<9;y++)for(unsigned x=0;x<9;x++)image.pixels[y*9+x]={u8(x*28),u8(y*28),u8((x+y)*13),255};
 std::vector<u8> memory(192,0xa5);Native::GpuResources resources;auto address=resources.addWritable(memory.data(),memory.size());
 Native::GxCommandState::CopyOperation copy{};copy.registers[0x49]=1|(2<<10);copy.registers[0x4a]=4|(4<<10);copy.registers[0x4b]=address>>5;copy.registers[0x4d]=3;copy.registers[0x52]=4<<4;
 writeRgb565Copy(image,copy,resources);
 std::vector<u8> compact(128);std::memcpy(compact.data(),memory.data(),64);std::memcpy(compact.data()+64,memory.data()+96,64);auto decoded=decodeGxTexture(BinaryView(compact),5,5,4,0,BinaryView(compact,0,0));
 for(unsigned y=0;y<5;y++)for(unsigned x=0;x<5;x++){auto p=image.pixels[(y+2)*9+x+1];u16 v=(u16(p.r>>3)<<11)|(u16(p.g>>2)<<5)|(p.b>>3);auto offset=std::size_t(y/4)*96+(x/4)*32+((y%4)*4+x%4)*2;assert(memory[offset]==u8(v>>8)&&memory[offset+1]==u8(v));auto expected=rgb565(v);auto actual=decoded.pixels[y*5+x];assert(actual.r==expected.r&&actual.g==expected.g&&actual.b==expected.b);}
 for(unsigned i=64;i<96;i++)assert(memory[i]==0xa5);assert(memory[191]==0xa5);
 assert(std::memcmp(resources.resolve(address,memory.size()),memory.data(),memory.size())==0);
 auto saved=memory;copy.registers[0x49]=8;bool rejected=false;try{writeRgb565Copy(image,copy,resources);}catch(const std::out_of_range&){rejected=true;}assert(rejected&&memory==saved);
 std::puts("PASS RGB565 offset rectangle tiled writes, padded stride, synchronized snapshot and source bounds");
}

