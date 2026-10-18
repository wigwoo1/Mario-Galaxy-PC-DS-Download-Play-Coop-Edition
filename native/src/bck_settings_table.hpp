#pragma once
#include "bck_settings.hpp"
#include "animation_clock.hpp"
#include "jmap_bridge.hpp"
#include "Game/Util/StringUtil.hpp"
class NativeBckSettingsTable {
 NativeJmapTable table;
public:
 BckCtrlData defaults;std::vector<BckCtrlData> entries;
 explicit NativeBckSettingsTable(BinaryView bytes):table(bytes){defaults.mName="_default";for(int row=0;row<table.info.getNumEntries();++row){JMapInfoIter iter(&table.info,row);BckCtrlData data;const char* name="";iter.getValue("name",&name);data.mName=name;auto read=[&](const char* key){s32 n=-1;iter.getValue(key,&n);assetCheck(n>=-32768&&n<=32767,"BCK setting outside signed 16-bit range");return s16(n);};data.mInterpole=read("interpole");data.mPlayFrame=read("play_frame");data.mStartFrame=read("start_frame");data.mEndFrame=read("end_frame");s32 attribute=-1;iter.getValue("attribute",&attribute);assetCheck(attribute<0||attribute<=255,"Invalid BCK attribute");data.mLoopMode=attribute<0?255:u8(attribute);if(MR::isEqualStringCase(name,"_default")){defaults=data;continue;}bool replaced=false;for(auto& entry:entries)if(MR::isEqualStringCase(entry.mName,name)){entry=data;replaced=true;break;}if(!replaced)entries.push_back(data);}}
 const BckCtrlData& select(const char* name)const{for(const auto& entry:entries)if(MR::isEqualStringCase(entry.mName,name)){bool valid=entry.mInterpole>=0||entry.mPlayFrame>=0||entry.mStartFrame>=0||entry.mEndFrame>=0||entry.mRepeatFrame>=0||entry.mLoopMode!=255;return valid?entry:defaults;}return defaults;}
};
inline int applyBckSettings(AnimationClock& clock,const BckCtrlData& data){
 XanimePlayer player;auto& ctrl=player.controller;ctrl.mStart=s16(clock.start);ctrl.mEnd=s16(clock.end);ctrl.mLoop=s16(clock.loop);ctrl.mFrame=clock.frame;ctrl.mRate=clock.rate;ctrl.mAttribute=clock.mode;ctrl.mState=clock.state;BckCtrlFunction::reflectBckCtrlData(data,&player);assetCheck(ctrl.mAttribute<=4&&ctrl.mEnd>ctrl.mStart&&ctrl.mLoop>=ctrl.mStart&&ctrl.mLoop<ctrl.mEnd&&std::isfinite(ctrl.mRate)&&std::abs(ctrl.mRate)<=4096,"Invalid authored BCK playback range/mode");clock.start=ctrl.mStart;clock.end=ctrl.mEnd;clock.loop=ctrl.mLoop;clock.frame=ctrl.mFrame;clock.rate=ctrl.mRate;clock.mode=ctrl.mAttribute;clock.state=ctrl.mState;return player.interpolation;
}
