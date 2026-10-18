#pragma once
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <utility>
#include <map>
// Byte-addressed big-endian traversal of the original relative-offset KCL octree.
// Owns its bytes; leaf results are prism indices rather than console pointers.
class NativeKclOctree {
 std::vector<std::uint8_t> bytes;
 mutable std::map<std::size_t,std::vector<std::uint16_t>> nativeLeaves;
 std::uint32_t word(std::size_t p)const{if(p>bytes.size()||bytes.size()-p<4)throw std::out_of_range("KCL octree word");return (std::uint32_t(bytes[p])<<24)|(std::uint32_t(bytes[p+1])<<16)|(std::uint32_t(bytes[p+2])<<8)|bytes[p+3];}
 std::uint16_t half(std::size_t p)const{if(p>bytes.size()||bytes.size()-p<2)throw std::out_of_range("KCL octree leaf");return (std::uint16_t(bytes[p])<<8)|bytes[p+1];}
public:
 explicit NativeKclOctree(std::vector<std::uint8_t> data):bytes(std::move(data)){}
 struct Leaf {int shift;std::vector<std::uint16_t> prisms;std::size_t offset;};
 Leaf lookup(std::uint32_t x,std::uint32_t y,std::uint32_t z,int widthShift,int xShift,int xyShift,std::size_t prismCount)const{
  if(widthShift<0||widthShift>31)throw std::invalid_argument("KCL block width shift");
  std::uint64_t index=0;
  if(xShift!=-1||xyShift!=-1){if(xShift<0||xShift>31||xyShift<0||xyShift>31)throw std::invalid_argument("KCL root shifts");index=(std::uint64_t(z>>widthShift)<<xyShift)|(std::uint64_t(y>>widthShift)<<xShift)|(x>>widthShift);}
  std::size_t base=0;std::uint64_t offset=index*4;int shift=widthShift;
  for(;;){if(offset>bytes.size()||base>bytes.size()-offset)throw std::out_of_range("KCL node offset");auto entry=word(base+std::size_t(offset));
   if(entry&0x80000000u){std::size_t relative=entry&0x7fffffffu;if(relative>bytes.size()-base)throw std::out_of_range("KCL leaf offset");std::size_t cursor=base+relative;
    // Original searchBlock returns the reserved halfword preceding the list.
    half(cursor);cursor+=2;Leaf result{shift,{},base+relative};
    for(;;){auto p=half(cursor);cursor+=2;if(!p)return result;if(p>prismCount)throw std::out_of_range("KCL prism index");result.prisms.push_back(p);}
   }
   if(shift==0||entry>bytes.size()-base)throw std::out_of_range("KCL child node");base+=entry;--shift;offset=((((z>>shift)&1)<<2)|(((y>>shift)&1)<<1)|((x>>shift)&1))*4;
  }
 }
 // Return the reserved halfword + native-endian prism list expected by original callers.
 // Stable identity is based on the disk leaf offset, preserving shared-leaf comparisons.
 struct NativeLeaf {int shift;const std::uint16_t* list;};
 NativeLeaf nativeLookup(std::uint32_t x,std::uint32_t y,std::uint32_t z,int widthShift,int xShift,int xyShift,std::size_t prismCount)const{
  auto decoded=lookup(x,y,z,widthShift,xShift,xyShift,prismCount);
  auto found=nativeLeaves.find(decoded.offset);
  if(found==nativeLeaves.end()){
   std::vector<std::uint16_t> list;list.reserve(decoded.prisms.size()+2);list.push_back(half(decoded.offset));
   list.insert(list.end(),decoded.prisms.begin(),decoded.prisms.end());list.push_back(0);
   found=nativeLeaves.emplace(decoded.offset,std::move(list)).first;
  }
  return {decoded.shift,found->second.data()};
 }
};
