#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <set>
#include <string>
#define NATIVE_JPA_REQUIRE(value) require((value), __LINE__)

namespace NativeJpa {
// Produce the representation used by the original typed JPA readers. Texture
// pixels/palettes and byte fields remain in their original GX representation.
inline std::vector<std::uint8_t> normalize(const void* data, std::size_t size) {
    using U8=std::uint8_t; using U16=std::uint16_t; using U32=std::uint32_t;
    const auto* s=static_cast<const U8*>(data);
    auto require=[](bool ok, unsigned line){if(!ok)throw std::runtime_error("Invalid JPAC2-10 particle archive at decoder line "+std::to_string(line));};
    NATIVE_JPA_REQUIRE(s && size>=16);
    NATIVE_JPA_REQUIRE(std::memcmp(s,"JPAC2-10",8)==0);
    auto extent=[&](std::size_t p,std::size_t n,std::size_t end){NATIVE_JPA_REQUIRE(p<=end && n<=end-p);};
    auto be16=[&](std::size_t p)->U16{extent(p,2,size);return U16(U16(s[p])<<8|s[p+1]);};
    auto be32=[&](std::size_t p)->U32{extent(p,4,size);return U32(s[p])<<24|U32(s[p+1])<<16|U32(s[p+2])<<8|s[p+3];};
    std::vector<U8> out(s,s+size);
    auto cv16=[&](std::size_t p){U16 v=be16(p);std::memcpy(out.data()+p,&v,2);};
    auto cv32=[&](std::size_t p){U32 v=be32(p);std::memcpy(out.data()+p,&v,4);};
    auto words=[&](std::size_t p,std::size_t end){for(;p<end;p+=4)cv32(p);};
    const U16 resources=be16(8),textures=be16(10);
    const std::size_t textureStart=be32(12);
    NATIVE_JPA_REQUIRE(textureStart>=16 && textureStart<=size);
    cv32(0);cv32(4);cv16(8);cv16(10);cv32(12);
    std::size_t pos=16;
    std::set<U16> ids;
    for(unsigned r=0;r<resources;++r){
        extent(pos,8,textureStart);
        NATIVE_JPA_REQUIRE(ids.insert(be16(pos)).second);
        unsigned blocks=be16(pos+2),fields=s[pos+4],keys=s[pos+5],tdb=s[pos+6];
        cv16(pos);cv16(pos+2);pos+=8;
        unsigned seenFields=0,seenKeys=0; bool dynamics=false,base=false,table=false;
        std::set<U32> singleton;
        for(unsigned b=0;b<blocks;++b){
            extent(pos,8,textureStart);
            U32 magic=be32(pos),length=be32(pos+4);
            NATIVE_JPA_REQUIRE(length>=8 && length%4==0);extent(pos,length,textureStart);
            auto blockExtent=[&](std::size_t offset,std::size_t n){extent(offset,n,length);};
            auto w=[&](std::size_t from,std::size_t end){blockExtent(from,end-from);words(pos+from,pos+end);};
            auto h=[&](std::size_t p){blockExtent(p,2);cv16(pos+p);};
            if(magic!=0x464c4431 && magic!=0x4b464131)NATIVE_JPA_REQUIRE(singleton.insert(magic).second);
            cv32(pos);cv32(pos+4);
            switch(magic){
            case 0x42454d31: // BEM1: dynamics
                blockExtent(0,0x7c);w(8,0x68);for(unsigned p=0x68;p<0x78;p+=2)h(p);dynamics=true;break;
            case 0x42535031: { // BSP1: base shape and optional animation tails
                blockExtent(0,0x34);U32 flags=be32(pos+8);
                w(8,12);h(12);h(14);w(16,24);h(24);h(36);
                std::size_t tail=0x34;
                if(flags&0x01000000){w(tail,tail+0x28);tail+=0x28;}
                if(s[pos+0x1e]&1)blockExtent(tail,s[pos+0x1f]);
                auto colors=[&](unsigned flag,unsigned offset,unsigned count){
                    if(!(s[pos+0x21]&flag))return;
                    U16 start=be16(pos+offset),maxFrame=be16(pos+0x24);
                    NATIVE_JPA_REQUIRE(count>0 && maxFrame<0x8000);blockExtent(start,std::size_t(count)*6);
                    U16 previous=0;
                    for(unsigned k=0;k<count;++k){U16 frame=be16(pos+start+k*6);if(!(frame<0x8000 && (k==0 || frame>previous)))throw std::runtime_error("JPA color key resource="+std::to_string(r)+" offset="+std::to_string(pos)+" start="+std::to_string(start)+" key="+std::to_string(k)+" frame="+std::to_string(frame)+" previous="+std::to_string(previous)+" max="+std::to_string(maxFrame));previous=frame;h(start+k*6);}
                };
                colors(2,12,s[pos+0x22]);colors(8,14,s[pos+0x23]);base=true;break;
            }
            case 0x45535031: // ESP1: extra shape
                blockExtent(0,0x60);w(8,0x28);h(0x28);h(0x2a);w(0x2c,0x60);break;
            case 0x53535031: // SSP1: child shape; colors are four individual bytes
                blockExtent(0,0x48);w(8,0x34);w(0x3c,0x40);h(0x40);h(0x42);h(0x46);break;
            case 0x45545831: // ETX1
                blockExtent(0,0x28);w(8,0x24);break;
            case 0x464c4431: // FLD1
                blockExtent(0,0x44);w(8,0x40);++seenFields;break;
            case 0x4b464131: // KFA1: byte key header then four floats per key
                blockExtent(0,12);NATIVE_JPA_REQUIRE(s[pos+9]>0);w(12,12+std::size_t(s[pos+9])*16);++seenKeys;break;
            case 0x54444231: // TDB1: texture lookup indices
                blockExtent(8,tdb*2);for(unsigned t=0;t<tdb;++t){NATIVE_JPA_REQUIRE(be16(pos+8+t*2)<textures);h(8+t*2);}table=true;break;
            default: throw std::runtime_error("Unsupported JPAC particle block");
            }
            pos+=length;
        }
        NATIVE_JPA_REQUIRE(seenFields==fields && seenKeys==keys && dynamics && base && (tdb==0 || table));
    }
    NATIVE_JPA_REQUIRE(pos<=textureStart);pos=textureStart;
    for(unsigned t=0;t<textures;++t){
        extent(pos,0x40,size);NATIVE_JPA_REQUIRE(be32(pos)==0x54455831); // TEX1
        std::size_t length=be32(pos+4);NATIVE_JPA_REQUIRE(length>=0x40);extent(pos,length,size);
        NATIVE_JPA_REQUIRE(std::memchr(s+pos+12,0,20)!=nullptr);
        const auto img=pos+0x20;
        NATIVE_JPA_REQUIRE(be16(img+2)>0 && be16(img+4)>0);
        auto imageOffset=be32(img+0x1c);if(imageOffset==0)imageOffset=0x20;
        extent(0x20+std::size_t(imageOffset),1,length);
        if(be16(img+10))extent(0x20+std::size_t(be32(img+12)),std::size_t(be16(img+10))*2,length);
        cv32(pos);cv32(pos+4);
        for(unsigned p:{2,4,10,26})cv16(img+p);
        cv32(img+12);cv32(img+28);
        pos+=length;
    }
    return out;
}
}

#undef NATIVE_JPA_REQUIRE
