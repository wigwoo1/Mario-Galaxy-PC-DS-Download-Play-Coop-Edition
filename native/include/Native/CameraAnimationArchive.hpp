#pragma once
#include <Native/CameraAnimationData.hpp>
#include <JSystem/JKernel/JKRFileLoader.hpp>
namespace Native {
inline bool decodeMountedCameraAnimation(CameraAnimationData& output,const uint8_t* resource) {
    if(!resource)return false;
    struct Lock {
        Lock(){OSLockMutex(&JKRFileLoader::sVolumeListMutex);}
        ~Lock(){OSUnlockMutex(&JKRFileLoader::sVolumeListMutex);}
    } lock;
    for(JSUPtrLink* current=JKRFileLoader::sVolumeList.mHead;current;current=current->mNext){
        auto* loader=static_cast<JKRFileLoader*>(current->mData);
        const s32 size=loader->getResSize(resource);
        if(size>=0)return output.decode(resource,static_cast<size_t>(size));
    }
    return false;
}
}
