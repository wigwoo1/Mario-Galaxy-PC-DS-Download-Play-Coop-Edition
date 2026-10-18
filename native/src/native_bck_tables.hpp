#pragma once
#include "animation.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
// Own all pointer targets required by J3DAnmTransformKey. Keep this owner alive
// and do not mutate its vectors while an original animation object is bound.
struct NativeBckTables {
    u8 loop,rotationShift;
    s16 duration;
    std::vector<J3DAnmTransformKeyTable> axes;
    std::vector<f32> scales,translations;
    std::vector<s16> rotations;
    void bind(J3DAnmTransformKey& animation){
        animation.mAttribute=loop;animation.mFrameMax=duration;animation.mFrame=0;
        animation.mDecShift=rotationShift;animation.mAnmTable=axes.data();
        animation.mScaleData=scales.data();animation.mRotData=rotations.data();animation.mTransData=translations.data();
    }
    explicit NativeBckTables(BinaryView view){
        auto validated=decodeBck(view);
        loop=validated.loop;rotationShift=validated.rotationShift;duration=s16(validated.duration);
        auto block=view.sub(32,view.u32(36));
        auto readFloats=[&](unsigned kind,std::vector<f32>& output){
            auto count=block.u16(14+kind*2);auto offset=block.u32(24+kind*4);
            extent(offset,size_t(count)*4,block.size());
            for(unsigned index=0;index<count;++index)output.push_back(block.f32(offset+index*4));
        };
        readFloats(0,scales);readFloats(2,translations);
        auto count=block.u16(16);auto offset=block.u32(28);
        extent(offset,size_t(count)*2,block.size());
        for(unsigned index=0;index<count;++index)rotations.push_back(s16(block.u16(offset+index*2)));
        size_t table=block.u32(20);axes.resize(validated.joints.size()*3);
        for(size_t axis=0;axis<axes.size();++axis){
            J3DAnmKeyTableBase* descriptors[]={&axes[axis].mScaleInfo,&axes[axis].mRotationInfo,&axes[axis].mTranslateInfo};
            for(unsigned kind=0;kind<3;++kind){size_t entry=table+axis*18+kind*6;
                *descriptors[kind]={block.u16(entry),block.u16(entry+2),block.u16(entry+4)};
            }
        }
    }
};
