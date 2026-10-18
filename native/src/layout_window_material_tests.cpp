#include <Native/LayoutMaterialResource.hpp>
#include "layout_resource.hpp"
#include <cassert>
#include <cstdio>
int main(int argc,char** argv){
 assert(argc==2);using namespace nw4r::lyt;
 static_assert(offsetof(res::WindowContent,materialIdx)==16&&sizeof(res::WindowContent)==20);
 alignas(res::WindowContent) std::array<u8,28> fixture{};fixture[16]=0x12;fixture[17]=0x34;fixture[24]=0xab;fixture[25]=0xcd;
 auto before=fixture;assert(Native::readLayoutWindowMaterialIndex(*reinterpret_cast<const res::WindowContent*>(fixture.data()))==0x1234);assert(fixture==before);
 WiiAssets disc(std::filesystem::u8path(argv[1]));unsigned checked=0,untextured=0,oldWrong=0;
 for(auto archive:{"/LayoutData/PointerGuidance.arc","/LayoutData/SysInfoWindow.arc","/LayoutData/InformationWindow.arc"}){
  Rarc arc(disc.readFile(archive));for(const auto& file:arc.files){auto bytes=arc.readFile(file.path);if(bytes.size()<16||std::memcmp(bytes.data(),"RLYT",4))continue;
   BinaryView data(bytes);auto layout=decodeLayoutResource(data);size_t offset=data.u16(12);
   for(unsigned i=0;i<data.u16(14);++i){auto length=data.u32(offset+4);auto block=data.sub(offset,length);
    if(block.tag(0)=="wnd1"){
     auto content=block.u32(96);auto expected=block.u16(content+16);auto actual=Native::readLayoutWindowMaterialIndex(*reinterpret_cast<const res::WindowContent*>(bytes.data()+offset+content));assert(actual==expected&&actual<layout.materials.size());++checked;
     if(content+26<=length&&block.u16(content+24)!=actual)++oldWrong;
     const auto& material=layout.materials.at(actual);
     if(block.u8(content+18)==0){assert(material.textures.empty()&&material.coordinateGenerators.empty());++untextured;std::printf("PASS authored untextured window %s selects material %u:%s, zero texture generators\n",layoutFixedName(block,12,16).c_str(),actual,material.name.c_str());}
    }offset+=length;
   }
  }
 }
 assert(checked==9&&untextured==2&&oldWrong>=5);
 std::printf("PASS original window material lookup: endian/offset fixture, immutable bytes, %u retail windows, %u originally wrong lookups, %u untextured pointer backgrounds\n",checked,oldWrong,untextured);
}