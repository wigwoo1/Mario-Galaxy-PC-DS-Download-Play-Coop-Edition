#include <Game/Speaker/SpkWave.hpp>
#include <cstring>
SpkWave::SpkWave() : mResource(nullptr) {
}
s32 SpkWave::getWaveSize(s32 wave) const {
    if (mResource == nullptr) {
        return 0;
    }

    return getWaveData(wave)->mSize;
}
u32 SpkWave::getLoopStartPos(s32 wave) const {
    if (mResource == nullptr) {
        return 0;
    }

    return getWaveData(wave)->mLoopStartPos;
}
u32 SpkWave::getLoopEndPos(s32 wave) const {
    if (mResource == nullptr) {
        return 0;
    }

    return getWaveData(wave)->mLoopEndPos;
}
s16* SpkWave::getWave(s32 wave) const {
    if (mResource == nullptr) {
        return nullptr;
    }

    return getWaveData(wave)->mWave;
}
WaveData* SpkWave::getWaveData(s32 wave) const {
    auto base=static_cast<unsigned char*>(mResource);u32 offset;std::memcpy(&offset,base+static_cast<size_t>(wave)*4+8,sizeof(offset));return reinterpret_cast<WaveData*>(base+offset);
}

