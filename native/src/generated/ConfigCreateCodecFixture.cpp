#include "Game/System/ConfigDataHolder.hpp"
#include "Game/System/ConfigDataMii.hpp"
#include "Game/System/ConfigDataMisc.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>
#include <cstdio>

ConfigDataCreateChunk::ConfigDataCreateChunk() : mIsCreated() {
    initializeData();
}

u32 ConfigDataCreateChunk::makeHeaderHashCode() const {
    return 0x2432DA;
}

u32 ConfigDataCreateChunk::getSignature() const {
    return 'CONF';
}

s32 ConfigDataCreateChunk::serialize(u8* buffer,u32 size) const {
 if(!buffer||size<1)return -1;
 buffer[0]=mIsCreated?0xff:0;
 return 1;
}
s32 ConfigDataCreateChunk::deserialize(const u8* buffer,u32 size){
 if(!buffer||size<1)return -1;
 mIsCreated=buffer[0]!=0;
 return 0;
}
void ConfigDataCreateChunk::initializeData() {
    mIsCreated = false;
}


