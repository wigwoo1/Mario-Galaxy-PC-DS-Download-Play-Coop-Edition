#include <cstring>
#include "Game/System/ConfigDataMisc.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

#define FLAG_LAST_LOADED_MARIO 0x1
#define FLAG_COMPLETE_ENDING_MARIO 0x2
#define FLAG_COMPLETE_ENDING_LUIGI 0x4

ConfigDataMisc::ConfigDataMisc() : mFlag(FLAG_LAST_LOADED_MARIO), mLastModified() {
    initializeData();
}

bool ConfigDataMisc::isLastLoadedMario() const {
    return !((mFlag & FLAG_LAST_LOADED_MARIO) - 1);
}

void ConfigDataMisc::setLastLoadedMario(bool lastLoadedMario) {
    if (lastLoadedMario) {
        mFlag |= FLAG_LAST_LOADED_MARIO;
    } else {
        mFlag &= ~FLAG_LAST_LOADED_MARIO;
    }
}

void ConfigDataMisc::onCompleteEndingMario() {
    mFlag |= FLAG_COMPLETE_ENDING_MARIO;
}

void ConfigDataMisc::onCompleteEndingLuigi() {
    mFlag |= FLAG_COMPLETE_ENDING_LUIGI;
}

bool ConfigDataMisc::isOnCompleteEndingMario() {
    return (mFlag & FLAG_COMPLETE_ENDING_MARIO) != 0;
}

bool ConfigDataMisc::isOnCompleteEndingLuigi() {
    return (mFlag & FLAG_COMPLETE_ENDING_LUIGI) != 0;
}

OSTime ConfigDataMisc::getLastModified() const {
    return mLastModified;
}

void ConfigDataMisc::updateLastModified() {
    mLastModified = OSGetTime();
}

u32 ConfigDataMisc::makeHeaderHashCode() const {
    return 0x1;
}

u32 ConfigDataMisc::getSignature() const {
    return 'MISC';
}

s32 ConfigDataMisc::serialize(u8* buffer,u32 size) const {
 static_assert(sizeof(OSTime)==8);
 if(!buffer||size<9)return -1;
 buffer[0]=mFlag;
 const u64 value=static_cast<u64>(mLastModified);
 for(int i=0;i<8;++i)buffer[i+1]=u8(value>>(56-i*8));
 return 9;
}
s32 ConfigDataMisc::deserialize(const u8* buffer,u32 size){
 if(!buffer||size<1||(size>1&&size<9))return -1;
 u64 value=0;
 if(size>=9)for(int i=0;i<8;++i)value=(value<<8)|buffer[i+1];
 OSTime timestamp;std::memcpy(&timestamp,&value,sizeof(timestamp));
 mFlag=buffer[0];mLastModified=timestamp;
 return 0;
}
void ConfigDataMisc::initializeData() {
    mFlag = FLAG_LAST_LOADED_MARIO;
    mLastModified = 0;
}

