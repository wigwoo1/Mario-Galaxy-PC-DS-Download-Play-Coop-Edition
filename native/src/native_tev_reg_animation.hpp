#pragma once
#include "native_name_table.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <array>
#include <memory>

// Keep the original J3D sampler and material attachment API. Only archive
// storage is converted: Wii offsets and integers never become host pointers.
class NativeTevRegAnimation final : public J3DAnmTevRegKey {
    std::vector<J3DAnmCRegKeyTable> colorTables;
    std::vector<J3DAnmKRegKeyTable> konstTables;
    std::array<std::vector<s16>,8> values;
    std::array<std::vector<u16>,2> ids;
    std::array<std::unique_ptr<NativeNameTable>,2> names;

    template<class Table>
    void readTables(BinaryView block, unsigned bank, std::vector<Table>& tables) {
        const size_t count=block.u16(0x0c+bank*2);
        const size_t tableOffset=block.u32(0x20+bank*4);
        const size_t idOffset=block.u32(0x28+bank*4);
        const size_t nameOffset=block.u32(0x30+bank*4);
        if(!count)return;
        assetCheck(tableOffset>=0x58 && idOffset>=0x58 && nameOffset>=0x58,"BRK metadata overlaps header");
        extent(tableOffset,count*28,block.size());
        extent(idOffset,count*2,block.size());
        extent(nameOffset,4,block.size());
        names[bank]=std::make_unique<NativeNameTable>(block.sub(nameOffset,block.size()-nameOffset));
        assetCheck(names[bank]->storage[0]==count,"BRK name and material counts differ");
        tables.resize(count);ids[bank].reserve(count);
        for(size_t i=0;i<count;++i){
            auto& table=tables[i];
            J3DAnmKeyTableBase* tracks[]={&table.mRTable,&table.mGTable,&table.mBTable,&table.mATable};
            for(unsigned c=0;c<4;++c){
                const size_t at=tableOffset+i*28+c*6;
                auto& track=*tracks[c];
                track={block.u16(at),block.u16(at+2),block.u16(at+4)};
                assetCheck(track.mType<=1,"Invalid BRK tangent type");
                const auto& channel=values[bank*4+c];
                const size_t stride=track.mType?4:3;
                extent(track.mOffset,track.mMaxFrame<=1?track.mMaxFrame:size_t(track.mMaxFrame)*stride,channel.size());
                for(size_t k=1;k<track.mMaxFrame;++k)
                    assetCheck(channel[track.mOffset+(k-1)*stride]<channel[track.mOffset+k*stride],"BRK key times are not increasing");
            }
            table.mColorId=block.u8(tableOffset+i*28+24);
            assetCheck(table.mColorId<4,"BRK register exceeds material animator capacity");
            ids[bank].push_back(block.u16(idOffset+i*2));
        }
    }
public:
    NativeTevRegAnimation(const void* data,size_t size){
        assetCheck(data && size>=40,"Missing or truncated BRK resource");
        const auto* bytes=static_cast<const u8*>(data);
        std::vector<u8> copy(bytes,bytes+size);BinaryView file(copy);
        assetCheck(file.tag(0)=="J3D1" && file.tag(4)=="brk1","Invalid BRK magic");
        const size_t declared=file.u32(8);
        assetCheck(declared>=40 && declared<=size && file.u32(12)==1,"Invalid BRK file header");
        const size_t length=file.u32(36);extent(32,length,declared);
        auto block=file.sub(32,length);
        assetCheck(length>=0x58 && block.tag(0)=="TRK1","Missing BRK register block");
        mAttribute=block.u8(8);mFrameMax=s16(block.u16(10));mFrame=0;
        assetCheck(mAttribute<=4 && mFrameMax>=0,"Invalid BRK playback metadata");
        for(unsigned c=0;c<8;++c){
            const size_t count=block.u16(0x10+c*2),offset=block.u32(0x38+c*4);
            if(count)assetCheck(offset>=0x58,"BRK values overlap header");
            extent(offset,count*2,length);values[c].reserve(count);
            for(size_t i=0;i<count;++i)values[c].push_back(s16(block.u16(offset+i*2)));
        }
        readTables(block,0,colorTables);readTables(block,1,konstTables);
        mCRegUpdateMaterialNum=u16(ids[0].size());mKRegUpdateMaterialNum=u16(ids[1].size());
        mCRegUpdateMaterialID=ids[0].data();mKRegUpdateMaterialID=ids[1].data();
        mAnmCRegKeyTable=colorTables.data();mAnmKRegKeyTable=konstTables.data();
        mCRegUpdateMaterialName.setResource(names[0]?names[0]->resource():nullptr);
        mKRegUpdateMaterialName.setResource(names[1]?names[1]->resource():nullptr);
        mCRegDataCountR=u16(values[0].size());mCRegDataCountG=u16(values[1].size());
        mCRegDataCountB=u16(values[2].size());mCRegDataCountA=u16(values[3].size());
        mKRegDataCountR=u16(values[4].size());mKRegDataCountG=u16(values[5].size());
        mKRegDataCountB=u16(values[6].size());mKRegDataCountA=u16(values[7].size());
        mAnmCRegDataR=values[0].data();mAnmCRegDataG=values[1].data();mAnmCRegDataB=values[2].data();mAnmCRegDataA=values[3].data();
        mAnmKRegDataR=values[4].data();mAnmKRegDataG=values[5].data();mAnmKRegDataB=values[6].data();mAnmKRegDataA=values[7].data();
    }
    NativeTevRegAnimation(const NativeTevRegAnimation&)=delete;
    NativeTevRegAnimation& operator=(const NativeTevRegAnimation&)=delete;
};
