#include <cstring>
#include "Game/System/ConfigDataMii.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>
#include <RFL_Types.h>

#define FLAG_UNK1 0x1
#define FLAG_UNK2 0x2

#define ICON_ID_MII 0
#define ICON_ID_MARIO 1
#define ICON_ID_LUIGI 2
#define ICON_ID_YOSHI 3
#define ICON_ID_KINOPIO 4
#define ICON_ID_PEACH 5

ConfigDataMii::ConfigDataMii() : mFlag(), mIconId(ICON_ID_MARIO), mMiiId() {
    mMiiId = new u8[sizeof(RFLCreateID)];

    initializeData();
}

void ConfigDataMii::setMiiOrIconId(const void* pMiiId, const u32* pIconId) {
    if (pMiiId != nullptr) {
        MR::copyMemory(mMiiId, pMiiId, sizeof(RFLCreateID));

        mFlag |= FLAG_UNK2;
        mIconId = ICON_ID_MII;
    } else {
        MR::fillMemory(mMiiId, 0, sizeof(RFLCreateID));

        mIconId = *pIconId;
    }
}

bool ConfigDataMii::getIconId(u32* pIconId) const {
    *pIconId = mIconId;

    return mIconId != ICON_ID_MII;
}

bool ConfigDataMii::getMiiId(void* pMiiId) const {
    MR::copyMemory(pMiiId, mMiiId, sizeof(RFLCreateID));

    return mIconId == ICON_ID_MII;
}

u32 ConfigDataMii::makeHeaderHashCode() const {
    return 0x2836E9;
}

u32 ConfigDataMii::getSignature() const {
    return 'MII ';
}

s32 ConfigDataMii::serialize(u8* buffer,u32 size) const {
 constexpr u32 idSize=sizeof(RFLCreateID);
 if(!buffer||size<idSize+2)return -1;
 buffer[0]=mFlag;std::memcpy(buffer+1,mMiiId,idSize);buffer[idSize+1]=mIconId;
 return idSize+2;
}
s32 ConfigDataMii::deserialize(const u8* buffer,u32 size){
 constexpr u32 idSize=sizeof(RFLCreateID);
 if(!buffer||size<idSize+1)return -1;
 const u8 icon=size==idSize+1?((buffer[0]&FLAG_UNK1)?ICON_ID_MII:ICON_ID_MARIO):buffer[idSize+1];
 mFlag=buffer[0];std::memcpy(mMiiId,buffer+1,idSize);mIconId=icon;
 return 0;
}
void ConfigDataMii::initializeData() {
    mFlag = 0;
    mIconId = ICON_ID_MARIO;

    MR::zeroMemory(mMiiId, sizeof(RFLCreateID));
}

