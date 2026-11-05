#include <Native/HeapPlacement.hpp>
#include "Game/RhythmLib/AudMeObject.hpp"

namespace {
AudMeHandle* allocateNativeMeHandles(u8 count, JKRHeap* heap) {
    if (!count) return nullptr;
    auto* handles = static_cast<AudMeHandle*>(JKRHeap::alloc(u32(count) * sizeof(AudMeHandle), alignof(AudMeHandle), heap));
    if (!handles) throw std::bad_alloc();
    for (u32 i = 0; i < count; ++i) new (handles + i) AudMeHandle();
    return handles;
}
}
AudMeObject::AudMeObject(TVec3f* pPos, u8 numHandles, JKRHeap* pHeap)
    : AudMeHandles(allocateNativeMeHandles(numHandles, pHeap), numHandles), mPos(pPos), mIsAllocated(true) {
}

AudMeObject::~AudMeObject() {
    if (mHandles) { for (int i = mNumHandles; i > 0; --i) mHandles[i - 1].~AudMeHandle(); JKRHeap::free(mHandles, nullptr); }
}

void AudMeObject::dispose() {
    for (u32 i = 0; i < mNumHandles; i++) {
        if (getHandle(i)->isMeAttached()) {
            getHandle(i)->releaseMe();
        }
    }
    mIsAllocated = false;
}

AudMeHandle* AudMeObject::startMe(u32 id) {
    if (!mIsAllocated) {
        return nullptr;
    }
    if (id == -1) {
        return nullptr;
    }

    AudMeHandle* handle = getHandleMeID(id);
    if (handle != nullptr && handle->isMeAttached() && handle->getMe()->mTimer == 0) {
        return nullptr;
    }

    if (handle == nullptr) {
        handle = getFreeHandle();
    }

    if (handle == nullptr) {
        handle = getLowestPrioHandle(id);
    }

    if (handle != nullptr) {
        AudMeMgr::getInstance()->startMe(id, handle, mPos);
    }

    return handle;
}

AudMeHandle* AudMeObject::getLowestPrioHandle(u32 id) {
    u8 lowestPrio = 0xFF;
    AudMeHandle* lowestPrioHandle = nullptr;
    for (s32 i = 0; i < mNumHandles; i++) {
        AudMe* me = getHandle(i)->getMe();
        if (me == nullptr) {
            return getHandle(i);
        }
        if (me->mMeId == 0xFFFFFFFF) {
            return getHandle(i);
        }

        u8 prio = AudMeMgr::getInstance()->getPriority(me->mMeId);
        if (prio < lowestPrio) {
            lowestPrio = prio;
            lowestPrioHandle = getHandle(i);
        }
    }

    if (AudMeMgr::getInstance()->getPriority(id) >= lowestPrio) {
        return lowestPrioHandle;
    }

    return nullptr;
}

