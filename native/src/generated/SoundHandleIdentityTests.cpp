#include <cstdint>
#include <cassert>
#include <cstdio>
// Lookup algorithm fixture; sound allocation/playback is outside this check.
struct Sound {std::uintptr_t identity;std::uintptr_t getUserData()const{return identity;}};
struct JAISoundHandle {Sound* sound;bool isSoundAttached()const{return sound!=nullptr;}Sound* operator->()const{return sound;}};
struct JAISoundHandles {JAISoundHandle* mHandles;int mNumHandles;JAISoundHandle* getHandleUserData(std::uintptr_t);};
JAISoundHandle* JAISoundHandles::getHandleUserData(std::uintptr_t addr) {
    for (int i = 0; i < mNumHandles; i++) {
        if (mHandles[i].isSoundAttached()) {
            if (mHandles[i]->getUserData() == addr) {
                return &mHandles[i];
            }
        }
    }

    return nullptr;
}
int main(){Sound first{0x100000001ull},second{0x200000001ull};JAISoundHandle handles[]={{nullptr},{&first},{&second}};JAISoundHandles table{handles,3};assert(table.getHandleUserData(first.identity)==&handles[1]);assert(table.getHandleUserData(second.identity)==&handles[2]);assert(table.getHandleUserData(1)==nullptr);handles[1].sound=nullptr;assert(table.getHandleUserData(first.identity)==nullptr);table.mNumHandles=0;assert(table.getHandleUserData(second.identity)==nullptr);std::puts("PASS native sound handle identity lookup: full-width identity, detached, missing and empty table (sound fixture)");}
