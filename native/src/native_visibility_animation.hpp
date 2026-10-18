#pragma once
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <JSystem/JKernel/JKRArchive.hpp>
namespace Native {
class OwnedVisibilityAnimation final : public J3DAnmVisibilityFull {
 std::vector<J3DAnmVisibilityFullTable> tables;
 std::vector<u8> values;
public:
 OwnedVisibilityAnimation(const void* data,size_t size) {
  auto* b=static_cast<const u8*>(data);
  auto check=[](bool ok){if(!ok)throw std::invalid_argument("Invalid BVA extent or visibility table");};
  check(b&&size>=32);check(!std::memcmp(b,"J3D1bva1",8));
  auto read32=[&](size_t o){check(o<=size&&size-o>=4);return u32(b[o])<<24|u32(b[o+1])<<16|u32(b[o+2])<<8|b[o+3];};
  auto read16=[&](size_t o){check(o<=size&&size-o>=2);return u16(u16(b[o])<<8|b[o+1]);};
  size_t declared=read32(8);check(declared>=32&&declared<=size);
  size_t cursor=32;bool found=false;
  for(u32 i=0;i<read32(12);++i){
   check(cursor<=declared&&declared-cursor>=8);size_t length=read32(cursor+4);
   check(length>=8&&length<=declared-cursor);
   if(!std::memcmp(b+cursor,"VAF1",4)){
    check(!found&&length>=24);found=true;
    mAttribute=b[cursor+8];mFrameMax=s16(read16(cursor+10));mFrame=0;
    mUpdateMaterialNum=read16(cursor+12);field_0xe=read16(cursor+14);
    size_t t=read32(cursor+16),v=read32(cursor+20);
    check(t>=24&&t<=length&&size_t(mUpdateMaterialNum)*4<=length-t);
    check(v>=24&&v<=length&&field_0xe<=length-v);
    values.assign(b+cursor+v,b+cursor+v+field_0xe);
    for(unsigned j=0;j<mUpdateMaterialNum;++j){
     u16 count=read16(cursor+t+j*4),offset=read16(cursor+t+j*4+2);
     check(count>0&&offset<=values.size()&&count<=values.size()-offset);
     tables.push_back({count,offset});
    }
   }
   cursor+=length;
  }
  check(found);mAnmTable=tables.data();mVisibility=values.data();
 }
 OwnedVisibilityAnimation(const OwnedVisibilityAnimation&)=delete;
 OwnedVisibilityAnimation& operator=(const OwnedVisibilityAnimation&)=delete;
};
inline J3DAnmBase* loadArchiveVisibility(const JKRArchive& archive,const void* data){
 if(!data)return nullptr;
 s32 size=archive.getResSize(data);
 if(size<32)throw std::invalid_argument("BVA resource not owned by archive or truncated");
 return new OwnedVisibilityAnimation(data,size_t(size));
}
inline std::unique_ptr<J3DAnmVisibilityFull> createVisibilityAnimation(const void* data,size_t size){
 return std::make_unique<OwnedVisibilityAnimation>(data,size);
}
}

