#include <Game/AudioLib/AudSoundObjHolder.hpp>
#include <cassert>
#include <cstdint>
#include <cstdio>
void AudSoundObjHolder::add(AudSoundObject* pSound) {
    if (mSize < mCapacity) {
        mArray[mSize] = pSound;
        mSize++;
    }
}
void AudSoundObjHolder::remove(AudSoundObject* pSound) {
    int soundIndex = -1;

    // Find sound object index in mArray
    for (int i = mSize - 1; i >= 0; i--) {
        if (mArray[i] == pSound) {
            soundIndex = i;
            break;
        }
    }

    // Remove sound object if found
    if (soundIndex >= 0) {
        moveOver(soundIndex, mSize - 1);
        mSize--;
    }
}
void AudSoundObjHolder::moveOver(s32 initialIndex, s32 finalIndex) {
    for (int i = initialIndex; i < finalIndex; i++) {
        mArray[i] = mArray[i + 1];
    }
}
// Storage constructor fixture; actual sound objects are never dereferenced.
AudSoundObjHolder::AudSoundObjHolder(JKRHeap*,s32 capacity):mArray(nullptr),mCapacity(capacity),mSize(0){}
int main(){AudSoundObject* storage[3]{};AudSoundObjHolder holder(nullptr,3);holder.mArray=storage;auto a=reinterpret_cast<AudSoundObject*>(std::uintptr_t(0x100000001ull));auto b=reinterpret_cast<AudSoundObject*>(std::uintptr_t(0x200000001ull));auto c=reinterpret_cast<AudSoundObject*>(std::uintptr_t(0x300000001ull));holder.add(a);holder.add(b);holder.add(c);holder.add(a);assert(holder.mSize==3);holder.remove(b);assert(holder.mSize==2&&storage[0]==a&&storage[1]==c);holder.remove(b);assert(holder.mSize==2);holder.add(a);holder.remove(a);assert(holder.mSize==2&&storage[0]==a&&storage[1]==c);holder.remove(a);holder.remove(c);assert(holder.mSize==0);holder.remove(c);assert(holder.mSize==0);std::puts("PASS original sound holder membership: capacity, full-width identity, compaction, duplicate-last removal and missing entries (storage fixture)");}
