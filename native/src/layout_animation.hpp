#pragma once
#include "graphics.hpp"
// Metadata only: original nw4r::lyt::res::BinaryFileHeader and AnimationBlock.
struct NativeLayoutAnimationMetadata{uint16_t frames;bool loop;uint16_t contents,files;};
inline NativeLayoutAnimationMetadata decodeLayoutAnimationMetadata(BinaryView data){
    assetCheck(data.size()>=16&&data.tag(0)=="RLAN"&&data.u16(4)==0xFEFF,"Invalid layout animation header");
    assetCheck(data.u32(8)==data.size()&&data.u16(12)>=16&&data.u16(12)<=data.size(),"Invalid layout animation file size");
    size_t offset=data.u16(12);bool found=false;NativeLayoutAnimationMetadata result{};
    for(unsigned i=0;i<data.u16(14);++i){extent(offset,8,data.size());auto size=data.u32(offset+4);assetCheck(size>=8,"Invalid layout animation block size");auto block=data.sub(offset,size);
        if(block.tag(0)=="pai1"){
            assetCheck(!found&&size>=20&&block.u8(10)<=1,"Invalid layout animation timing block");
            result={block.u16(8),block.u8(10)!=0,block.u16(14),block.u16(12)};
            assetCheck(result.frames>0,"Empty layout animation duration");
            extent(20,uint64_t(result.files)*4,size);auto table=block.u32(16);if(result.contents)extent(table,uint64_t(result.contents)*4,size);
            for(unsigned j=0;j<result.contents;++j){auto content=block.u32(table+j*4);assetCheck(content>=20&&content<size,"Invalid layout animation content offset");}
            found=true;
        }offset+=size;
    }
    assetCheck(found&&offset==data.size(),"Missing layout timing block or trailing data");return result;
}
inline void layoutAnimationMetadataTests(){
    std::vector<uint8_t> bytes(36);std::memcpy(bytes.data(),"RLAN",4);bytes[4]=0xFE;bytes[5]=0xFF;bytes[11]=36;bytes[13]=16;bytes[15]=1;std::memcpy(bytes.data()+16,"pai1",4);bytes[23]=20;bytes[25]=30;
    auto info=decodeLayoutAnimationMetadata(BinaryView(bytes));assetCheck(info.frames==30&&!info.loop,"Layout timing metadata mismatch");bytes[26]=1;assetCheck(decodeLayoutAnimationMetadata(BinaryView(bytes)).loop,"Layout loop metadata mismatch");
    for(int failure=0;failure<3;++failure){auto bad=bytes;if(failure==0)bad.pop_back();if(failure==1)bad[26]=2;if(failure==2)bad[23]=7;bool rejected=false;try{decodeLayoutAnimationMetadata(BinaryView(bad));}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Invalid layout metadata accepted");}
}
struct NativeLayoutKey{float frame,value,slope;};
struct NativeLayoutTrack{std::string name,kind;uint8_t contentType,id,target,curve;std::vector<NativeLayoutKey> keys;};
struct NativeLayoutAnimation{NativeLayoutAnimationMetadata metadata;std::vector<NativeLayoutTrack> tracks;};
inline NativeLayoutAnimation decodeLayoutAnimation(BinaryView data){
    NativeLayoutAnimation result{decodeLayoutAnimationMetadata(data),{}};size_t offset=data.u16(12);
    for(unsigned b=0;b<data.u16(14);++b){auto block=data.sub(offset,data.u32(offset+4));offset+=block.size();if(block.tag(0)!="pai1")continue;auto table=block.u32(16);
        for(unsigned c=0;c<result.metadata.contents;++c){auto contentOffset=block.u32(table+c*4);auto content=block.sub(contentOffset,block.size()-contentOffset);extent(0,24,content.size());std::string name;for(unsigned n=0;n<20&&content.u8(n);++n)name.push_back(char(content.u8(n)));assetCheck(!name.empty(),"Empty animation content name");auto infoCount=content.u8(20),type=content.u8(21);assetCheck(type<=1,"Invalid animation content type");extent(24,uint64_t(infoCount)*4,content.size());
            for(unsigned i=0;i<infoCount;++i){auto infoOffset=content.u32(24+i*4);assetCheck(infoOffset>=24+infoCount*4,"Animation info overlaps header");auto info=content.sub(infoOffset,content.size()-infoOffset);extent(0,8,info.size());auto count=info.u8(4);extent(8,uint64_t(count)*4,info.size());
                for(unsigned t=0;t<count;++t){auto targetOffset=info.u32(8+t*4);assetCheck(targetOffset>=8+count*4,"Animation target overlaps header");auto target=info.sub(targetOffset,info.size()-targetOffset);extent(0,12,target.size());NativeLayoutTrack track{name,info.tag(0),type,target.u8(0),target.u8(1),target.u8(2),{}};auto keys=target.u16(4);assetCheck(keys>0&&(track.curve==1||track.curve==2),"Unsupported layout animation curve");auto keyOffset=target.u32(8);assetCheck(keyOffset>=12,"Animation keys overlap target");size_t stride=track.curve==2?12:8;extent(keyOffset,uint64_t(keys)*stride,target.size());
                    for(unsigned k=0;k<keys;++k){auto p=keyOffset+k*stride;NativeLayoutKey key{target.f32(p),track.curve==2?target.f32(p+4):float(target.u16(p+4)),track.curve==2?target.f32(p+8):0.0f};assetCheck((track.keys.empty()||key.frame>=track.keys.back().frame),"Invalid layout key order/frame");track.keys.push_back(key);}assetCheck(result.tracks.size()<4096,"Excess layout animation tracks");result.tracks.push_back(std::move(track));
                }
            }
        }
    }return result;
}
// Native Hermite/step evaluation; duplicate frames retain incoming/outgoing tangents.
inline float sampleLayoutTrack(const NativeLayoutTrack& track,float frame){
    assetCheck(std::isfinite(frame)&&!track.keys.empty(),"Invalid layout animation sample");const auto& keys=track.keys;if(frame<keys.front().frame)return keys.front().value;
    auto upper=std::upper_bound(keys.begin(),keys.end(),frame,[](float f,const NativeLayoutKey& key){return f<key.frame;});if(upper==keys.end())return keys.back().value;auto lower=upper-1;if(track.curve==1)return lower->value;
    float span=upper->frame-lower->frame;assetCheck(span>0,"Empty layout interpolation interval");float t=(frame-lower->frame)/span,t2=t*t,t3=t2*t;float value=(2*t3-3*t2+1)*lower->value+(t3-2*t2+t)*span*lower->slope+(-2*t3+3*t2)*upper->value+(t3-t2)*span*upper->slope;assetCheck(std::isfinite(value),"Layout interpolation overflow");return value;
}
inline void layoutTrackTests(){
    NativeLayoutTrack track{"test","RLPA",0,0,0,2,{{0,0,1},{10,10,1}}};assetCheck(sampleLayoutTrack(track,5)==5&&sampleLayoutTrack(track,-1)==0&&sampleLayoutTrack(track,11)==10,"Layout Hermite interpolation/bounds mismatch");track.keys={{0,0,0},{5,2,0},{5,2,1},{10,7,1}};assetCheck(sampleLayoutTrack(track,5)==2&&sampleLayoutTrack(track,7.5f)==4.5f,"Layout duplicate-frame outgoing tangent mismatch");track.curve=1;assetCheck(sampleLayoutTrack(track,4)==0&&sampleLayoutTrack(track,5)==2,"Layout step interpolation mismatch");
}

