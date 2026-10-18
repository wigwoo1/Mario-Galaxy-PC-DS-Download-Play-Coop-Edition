#include "Game/Map/StageSwitch.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/JMapIdInfo.hpp"

bool BitFlag128::get(int idx) const {
    u32 flags = mFlags[idx / 32];
    u32 mask = (1U << (idx % 32));
    return mask == (flags & static_cast<u32>(mask));
}

void BitFlag128::set(int bitIndex, bool setTrue) {
    u32 valIdx = bitIndex / 32;
    u32& flag = mFlags[valIdx];

    if (setTrue) {
        flag |= (1U << (bitIndex % 32));
    } else {
        flag &= ~(1U << (bitIndex % 32));
    }
}

ZoneSwitch::ZoneSwitch() : BitFlag128() {
    s32 idx = 0;

    while (idx < 0x80) {
        set(idx, false);
        idx++;
    }
}


