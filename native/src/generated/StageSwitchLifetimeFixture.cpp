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


StageSwitchContainer::StageSwitchContainer() : NameObj("ステージスイッチ"), mSwitches(), mGlobalSwitches() {
    mGlobalSwitches = new ZoneSwitch();
}
StageSwitchContainer::~StageSwitchContainer() { for (int i=0;i<mSwitches.size();++i) delete mSwitches[i].mSwitch; delete mGlobalSwitches; }
SwitchIdInfo::SwitchIdInfo(s32 switchID, const JMapInfoIter& rIter) : mIDInfo(), mIsGlobal(switchID >= 1000) {
    mIDInfo = new JMapIdInfo(switchID, rIter);
}
s32 SwitchIdInfo::getSwitchNo() const {
    if (mIsGlobal) {
        return mIDInfo->_0 - 1000;
    }

    return mIDInfo->_0;
}
void StageSwitchContainer::createAndAddZone(const SwitchIdInfo& rInfo) {
    if (findZoneSwitchFromTable(rInfo) != nullptr) {
        return;
    }

    ZoneSwitch* pZoneSwitch = new ZoneSwitch();
    ContainerSwitch sw;
    sw.mZoneId = rInfo.mIDInfo->mZoneID;
    sw.mSwitch = pZoneSwitch;
    mSwitches.push_back(sw);
}
ZoneSwitch* StageSwitchContainer::getZoneSwitch(const SwitchIdInfo& rInfo) {
    if (rInfo.mIsGlobal) {
        return mGlobalSwitches;
    }

    return findZoneSwitchFromTable(rInfo);
}
ZoneSwitch* StageSwitchContainer::findZoneSwitchFromTable(const SwitchIdInfo& rInfo) {
    for (int i = 0; i < mSwitches.size(); i++) {
        ContainerSwitch* pContainer = &mSwitches[i];

        if (pContainer->mZoneId == rInfo.mIDInfo->mZoneID) {
            return pContainer->mSwitch;
        }
    }

    return nullptr;
}
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/SceneUtil.hpp"

JMapIdInfo::JMapIdInfo(s32 a1, const JMapInfoIter& rIter) {
    mZoneID = -1;
    initialize(a1, rIter);
}

JMapIdInfo::JMapIdInfo(s32 a1, s32 zoneId) {
    _0 = a1;
    mZoneID = zoneId;
}

void JMapIdInfo::initialize(s32 a1, const JMapInfoIter& rIter) {
    _0 = a1;
    mZoneID = MR::getPlacedZoneId(rIter);
}


