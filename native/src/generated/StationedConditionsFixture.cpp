#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <Game/System/GameSystemStationedArchiveLoader.hpp>
#include <Game/System/StationedFileInfo.hpp>
StationedArchiveLoader::Condition::~Condition() {
}

JKRHeap* StationedArchiveLoader::Condition::getProperHeap(const MR::StationedFileInfo* pInfo) const {
    return nullptr;
}

bool ConditionIfIsNotPlayer::isExecute(const MR::StationedFileInfo* pInfo) const {
    return pInfo->mLoadType != MR::StationedFileInfo::LOAD_TYPE_MOUNT_RESOURCE_MARIO &&
           pInfo->mLoadType != MR::StationedFileInfo::LOAD_TYPE_MOUNT_RESOURCE_LUIGI;
}

ConditionUsePlayerHeap::ConditionUsePlayerHeap() : mNapaHeap(), mGDDRHeap(), mIsDataMario(true) {
}

bool ConditionUsePlayerHeap::isExecute(const MR::StationedFileInfo* pInfo) const {
    MR::StationedFileInfo::LoadType type = MR::StationedFileInfo::LOAD_TYPE_MOUNT_RESOURCE_LUIGI;

    if (mIsDataMario) {
        type = MR::StationedFileInfo::LOAD_TYPE_MOUNT_RESOURCE_MARIO;
    }

    return pInfo->mLoadType == type;
}

JKRHeap* ConditionUsePlayerHeap::getProperHeap(const MR::StationedFileInfo* pInfo) const {
    switch (pInfo->mHeapType) {
    case MR::StationedFileInfo::HEAP_TYPE_NAPA:
        return mNapaHeap;
    case MR::StationedFileInfo::HEAP_TYPE_GDDR:
        return mGDDRHeap;
    case MR::StationedFileInfo::HEAP_TYPE_NONE:
        return nullptr;
    default:
        return nullptr;
    }
}

bool ConditionIsEqualType::isExecute(const MR::StationedFileInfo* pStationedFileInfo) const {
    return pStationedFileInfo->mLoadType == mLoadType;
}

