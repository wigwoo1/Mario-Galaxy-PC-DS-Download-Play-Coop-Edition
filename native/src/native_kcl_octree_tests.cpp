#include "native_kcl_storage.hpp"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>
int main(int argc,char** argv){
 std::vector<std::uint8_t> root={0x80,0,0,4,0,0,0,1,0,2,0,0};NativeKclOctree tree(root);auto leaf=tree.lookup(100,200,300,4,-1,-1,2);assert(leaf.shift==4 && leaf.prisms==std::vector<std::uint16_t>({1,2}));
 bool rejected=false;try{tree.lookup(0,0,0,4,-1,-1,1);}catch(const std::out_of_range&){rejected=true;}assert(rejected);
 std::vector<std::uint8_t> nested(44,0);nested[3]=4;for(int i=0;i<8;++i){nested[4+i*4]=0x80;nested[7+i*4]=32;}nested[39]=1;NativeKclOctree child(nested);leaf=child.lookup(1,1,1,1,-1,-1,1);assert(leaf.shift==0 && leaf.prisms.size()==1 && leaf.prisms[0]==1);
 auto stableA=child.nativeLookup(0,0,0,1,-1,-1,1);auto stableB=child.nativeLookup(1,1,1,1,-1,-1,1);
 assert(stableA.list==stableB.list && stableA.list[0]==0 && stableA.list[1]==1 && stableA.list[2]==0);
 NativeKclOctree childCopy(child);auto copied=childCopy.nativeLookup(1,1,1,1,-1,-1,1);assert(copied.list!=stableA.list && copied.list[1]==1);
 root.pop_back();rejected=false;try{NativeKclOctree(root).lookup(0,0,0,4,-1,-1,2);}catch(const std::out_of_range&){rejected=true;}assert(rejected);
 rejected=false;try{tree.lookup(0,0,0,32,-1,-1,2);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 if(argc==2){
  std::ifstream input(argv[1],std::ios::binary);if(!input)throw std::runtime_error("KCL input not found");
  std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(input)),{});
  auto word=[&](std::size_t p){if(p>data.size()||data.size()-p<4)throw std::out_of_range("KCL header");return (std::uint32_t(data[p])<<24)|(std::uint32_t(data[p+1])<<16)|(std::uint32_t(data[p+2])<<8)|data[p+3];};
  NativeKclStorage storage(data);NativeKclStorage copy(storage);assert(copy.positions.data()!=storage.positions.data());assert(copy.prisms.data()!=storage.prisms.data());
  auto prisms=word(8),octree=word(12);if(octree<=prisms||octree>=data.size()||(octree-prisms)%16)throw std::runtime_error("Invalid prism/octet offsets");
  auto count=(octree-prisms)/16-1;NativeKclOctree real(std::vector<std::uint8_t>(data.begin()+octree,data.end()));
  assert(storage.triangleCount()==count);for(std::size_t i=1;i<storage.prisms.size();++i)assert(storage.prisms[i].mAttribute==std::uint16_t((data[prisms+i*16+14]<<8)|data[prisms+i*16+15]));
  auto maskX=~word(32),maskY=~word(36),maskZ=~word(40);std::uint32_t rng=12345;std::size_t references=0;
  for(int i=0;i<4096;++i){auto next=[&](){rng=rng*1664525u+1013904223u;return rng;};auto x=next()&maskX,y=next()&maskY,z=next()&maskZ;
   auto sample=real.lookup(x,y,z,int(word(44)),std::int32_t(word(48)),std::int32_t(word(52)),count);references+=sample.prisms.size();auto native=storage.octree.nativeLookup(x,y,z,storage.widthShift,storage.xShift,storage.xyShift,storage.triangleCount());assert(native.shift==sample.shift);for(std::size_t j=0;j<sample.prisms.size();++j)assert(native.list[j+1]==sample.prisms[j]);assert(native.list[sample.prisms.size()+1]==0);}
  std::printf("PASS extracted KCL: 4096 bounded cell lookups, %zu prism references, %u prisms\n",references,count);
 }
 std::puts("PASS native KCL root/child traversal, prism bounds and truncated leaf rejection");
}
