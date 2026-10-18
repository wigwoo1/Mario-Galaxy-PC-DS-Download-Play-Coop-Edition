#include <Game/NameObj/NameObjListExecutor.hpp>
#include <Game/System/DrawBufferGroup.hpp>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <new>
static void* arrays[32];
static unsigned allocated, released;
void* operator new[](std::size_t size) {
    void* pointer=std::malloc(size);if(!pointer)throw std::bad_alloc();
    assert(allocated<32);arrays[allocated++]=pointer;return pointer;
}
void operator delete[](void* pointer) noexcept {
    if(!pointer)return;
    unsigned index=0;while(index<allocated&&arrays[index]!=pointer)++index;
    assert(index<allocated && "Array cleanup must release allocation base, not element pointer");
    arrays[index]=nullptr;++released;std::free(pointer);
}
void operator delete[](void* pointer,std::size_t) noexcept {::operator delete[](pointer);}
// Constructor/service boundaries outside the actual executor cleanup under test.
DrawBufferHolder::DrawBufferHolder() : mTableInitialized(false) {}
DrawBufferGroup::DrawBufferGroup() : mDrawCameraType(0),mLightType(-1),mLightLoadType(-1) {}
NameObjCategoryList::~NameObjCategoryList() {}
NameObjCategoryList::CategoryInfo::~CategoryInfo() {}
void NameObjListExecutor::initMovementList() {}
void NameObjListExecutor::initCalcAnimList() {}
void NameObjListExecutor::initCalcViewAndEntryList() {}
void NameObjListExecutor::initDrawList() {}
int main() {
    {
        NameObjListExecutor executor;
        executor.mBufferHolder=new DrawBufferHolder;
        executor.mBufferHolder->mBufferGroups.init(3);
        for(auto* group=executor.mBufferHolder->mBufferGroups.begin();group!=executor.mBufferHolder->mBufferGroups.end();++group){
            group->mExecutors.init(2);group->mActiveExecutors.init(1);
        }
        assert(allocated==7);
    }
    assert(released==allocated);
    std::puts("PASS actual scene executor cleanup: correct array allocation base and all nested group arrays released");
}
