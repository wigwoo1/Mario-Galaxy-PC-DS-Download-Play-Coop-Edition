#include <Native/BckAnimation.hpp>
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
void putBE16(std::vector<u8>& bytes,size_t offset,u16 value){bytes.at(offset)=u8(value>>8);bytes.at(offset+1)=u8(value);}
void putBE32(std::vector<u8>& bytes,size_t offset,u32 value){bytes.at(offset)=u8(value>>24);bytes.at(offset+1)=u8(value>>16);bytes.at(offset+2)=u8(value>>8);bytes.at(offset+3)=u8(value);}
void putBEFloat(std::vector<u8>& bytes,size_t offset,float value){u32 bits;std::memcpy(&bits,&value,sizeof(bits));putBE32(bytes,offset,bits);}
}
int main(){
    {
        constexpr size_t size=124,block=32;
        std::vector<u8> bytes(size);std::memcpy(bytes.data(),"J3D1bca1",8);putBE32(bytes,8,size);putBE32(bytes,12,1);
        std::memcpy(bytes.data()+block,"ANF1",4);putBE32(bytes,block+4,92);bytes[block+8]=2;
        putBE16(bytes,block+10,10);putBE16(bytes,block+12,1);putBE32(bytes,block+20,36);putBE32(bytes,block+24,72);putBE32(bytes,block+28,80);putBE32(bytes,block+32,84);
        for(size_t axis=0;axis<3;++axis){size_t entry=block+36+axis*12;putBE16(bytes,entry,2);putBE16(bytes,entry+4,2);putBE16(bytes,entry+8,2);}
        putBEFloat(bytes,block+72,1);putBEFloat(bytes,block+76,2);putBE16(bytes,block+80,0);putBE16(bytes,block+82,100);putBEFloat(bytes,block+84,10);putBEFloat(bytes,block+88,20);
        std::unique_ptr<J3DAnmBase> owned(Native::createBcaAnimation(bytes.data(),bytes.size()));assert(owned&&owned->getKind()==9);
        auto* animation=static_cast<J3DAnmTransformFull*>(owned.get());animation->mFrame=1;J3DTransformInfo pose;animation->getTransform(0,&pose);
        assert(pose.mScale.x==2&&pose.mScale.y==2&&pose.mScale.z==2&&pose.mRotation.x==100&&pose.mTranslate.x==20);
        std::fill(bytes.begin(),bytes.end(),0xdd);animation->getTransform(0,&pose);assert(pose.mScale.x==2&&pose.mTranslate.z==20);
        auto malformed=std::vector<u8>(size);std::memcpy(malformed.data(),"J3D1bca1",8);putBE32(malformed,8,size);putBE32(malformed,12,1);std::memcpy(malformed.data()+block,"ANF1",4);putBE32(malformed,block+4,92);putBE16(malformed,block+10,10);putBE16(malformed,block+12,1);putBE32(malformed,block+20,0xfffffff0u);
        bool rejected=false;try{std::unique_ptr<J3DAnmBase> invalid(Native::createBcaAnimation(malformed.data(),malformed.size()));}catch(const std::runtime_error&){rejected=true;}assert(rejected);
        std::puts("PASS owned big-endian BCA transform tables, playback sample, source lifetime and malformed-offset rejection");
    }
    constexpr size_t block=32,table=block+0x34,ids=table+24,names=ids+2,values=names+12,fileSize=values+8;
    std::vector<u8> bytes(fileSize);
    std::memcpy(bytes.data(),"J3D1bpk1",8);putBE32(bytes,8,fileSize);putBE32(bytes,12,1);
    std::memcpy(bytes.data()+block,"PAK1",4);putBE32(bytes,block+4,fileSize-block);
    bytes[block+8]=2;putBE16(bytes,block+12,10);putBE16(bytes,block+14,1);
    for(unsigned channel=0;channel<4;++channel)putBE16(bytes,block+0x10+channel*2,1);
    putBE32(bytes,block+0x18,0x34);putBE32(bytes,block+0x1c,ids-block);putBE32(bytes,block+0x20,names-block);
    for(unsigned channel=0;channel<4;++channel)putBE32(bytes,block+0x24+channel*4,values-block+channel*2);
    for(unsigned channel=0;channel<4;++channel){size_t entry=table+channel*6;putBE16(bytes,entry,1);putBE16(bytes,entry+2,0);putBE16(bytes,entry+4,0);}
    putBE16(bytes,ids,0);putBE16(bytes,names,1);putBE16(bytes,names+2,0);putBE16(bytes,names+4,1388);putBE16(bytes,names+6,8);
    std::memcpy(bytes.data()+names+8,"mat",4);
    for(unsigned channel=0;channel<4;++channel)putBE16(bytes,values+channel*2,channel==3?255:(channel+1)*10);

    std::unique_ptr<J3DAnmBase> owned(Native::createBpkAnimation(bytes.data(),bytes.size()));
    assert(owned&&owned->getKind()==11);
    auto* animation=static_cast<J3DAnmColorKey*>(owned.get());
    GXColor color{};animation->getColor(0,&color);assert(color.r==10&&color.g==20&&color.b==30&&color.a==255);
    assert(animation->mUpdateMaterialName.getIndex("mat")==0);
    std::fill(bytes.begin(),bytes.end(),0xdd);color={};animation->getColor(0,&color);assert(color.r==10&&color.g==20&&color.b==30&&color.a==255);

    auto malformed=std::vector<u8>(fileSize);std::memcpy(malformed.data(),"J3D1bpk1",8);putBE32(malformed,8,fileSize);putBE32(malformed,12,1);
    std::memcpy(malformed.data()+block,"PAK1",4);putBE32(malformed,block+4,fileSize-block);putBE16(malformed,block+14,1);putBE32(malformed,block+0x18,0xfffffff0u);
    bool rejected=false;try{std::unique_ptr<J3DAnmBase> invalid(Native::createBpkAnimation(malformed.data(),malformed.size()));}catch(const std::runtime_error&){rejected=true;}
    assert(rejected);
    std::puts("PASS owned big-endian BPK color tables, material names, playback sample, source lifetime and malformed-offset rejection");
}
