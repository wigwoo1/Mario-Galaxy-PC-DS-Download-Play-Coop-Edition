#pragma once
#include "native_scene.hpp"
#include "jmap_bridge.hpp"
#include "stage_placement.hpp"
// A native placement adapter using original NameObj/JMapInfo lifecycle; not a game actor factory.
class NativePlacedActor:public NameObj {
    std::shared_ptr<NativeJmapTable> table;std::string label;
public:
    StagePlacement placement;AffineMatrix parent,world;bool initialized=false,afterPlacement=false;size_t updates=0,animations=0,views=0;mutable size_t draws=0;
    NativePlacedActor(std::shared_ptr<NativeJmapTable> data,const AffineMatrix& matrix):NameObj("native placed actor"),table(std::move(data)),parent(matrix){}
    void init(const JMapInfoIter& iterator)override{
        assetCheck(iterator.isValid()&&iterator.mInfo==&table->info,"Invalid placed actor iterator");
        int type=table->info.getValueType("name");assetCheck(type==1||type==6,"Invalid placed actor name type");const char* name=nullptr;assetCheck(iterator.getValue("name",&name),"Missing placed actor name");label=name;placement.name=label;placement.row=size_t(iterator.mIndex);setName(label.c_str());
        auto number=[&](const char* key,float fallback){int type=table->info.getValueType(key);if(type==7)return fallback;assetCheck(type==2,"Invalid placed actor transform type");float value=0;assetCheck(iterator.getValue(key,&value)&&std::isfinite(value),"Invalid placed actor transform");return value;};
        const char* pos[]={"pos_x","pos_y","pos_z"},*rot[]={"dir_x","dir_y","dir_z"},*scale[]={"scale_x","scale_y","scale_z"};
        for(size_t k=0;k<3;++k){placement.position[k]=number(pos[k],0);placement.rotationDegrees[k]=number(rot[k],0);placement.scale[k]=number(scale[k],1);}for(size_t i=0;i<placement.arguments.size();++i){auto key="Obj_arg"+std::to_string(i);int type=table->info.getValueType(key.c_str());if(type!=7){assetCheck(type==0||type==3||type==4,"Invalid placed actor argument type");s32 value=-1;assetCheck(iterator.getValue(key.c_str(),&value),"Invalid placed actor argument");placement.arguments[i]=value;}}for(size_t i=0;i<stageSwitchFields.size();++i){auto key=stageSwitchFields[i];int type=table->info.getValueType(key);if(type!=7){assetCheck(type==0||type==3||type==4,"Invalid placed actor switch type");s32 value=-1;assetCheck(iterator.getValue(key,&value),"Invalid placed actor switch");placement.switches[i]=value;}}world=concatenate(parent,placementMatrix(placement));initialized=true;
    }
    void initAfterPlacement()override{assetCheck(initialized,"Actor after-placement before initialization");afterPlacement=true;}
    void movement()override{assetCheck(afterPlacement,"Uninitialized placed actor movement");++updates;}
    void calcAnim()override{assetCheck(afterPlacement,"Uninitialized placed actor animation");++animations;}
    void calcViewAndEntry()override{assetCheck(afterPlacement,"Uninitialized placed actor view");++views;}
    void draw()const override{assetCheck(afterPlacement,"Uninitialized placed actor draw");++draws;}
};


