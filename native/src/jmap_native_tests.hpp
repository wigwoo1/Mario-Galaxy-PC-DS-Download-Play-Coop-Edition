#pragma once
#include "Game/Util/JMapInfo.hpp"
#include "JSystem/JGadget/hashcode.hpp"
inline void nativeJmapTests(){
    struct HostTable {s32 rows,fields,offset;u32 stride;JMapItem items[2];struct Row {s32 argument;f32 position;} records[2];};
    HostTable table{2,2,40,8,{{JGadget::getHashCode("Obj_arg0"),0xffffffff,0,0,0},{JGadget::getHashCode("pos_x"),0xffffffff,4,0,2}},{{-1,12.5f},{7,-20.25f}}};
    static_assert(offsetof(HostTable,records)==40&&sizeof(JMapItem)==12);
    JMapInfo info;assetCheck(info.attach(&table)&&info.getNumEntries()==2&&info.getNumFields()==2,"Original JMapInfo host attach failed");
    s32 argument=0;f32 position=0;assetCheck(info.getValue(0,"Obj_arg0",&argument)&&argument==-1&&info.getValue(1,"pos_x",&position)&&position==-20.25f,"Original JMapInfo signed/float lookup failed");
    auto found=info.findElement<s32>("Obj_arg0",7,0);assetCheck(found.isValid()&&found.mIndex==1&&info.searchItemInfo("missing")==-1&&JGadget::getHashCode("pos_x")==jmapHash("pos_x"),"Original JMap lookup/hash test failed");
}
