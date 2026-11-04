#pragma once
#include <Native/HeapPlacement.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JKernel/JKRDisposer.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <stdexcept>

namespace Native {
class AudioPoolHeapBinding;
// Pool/heap construction and disposal must be serialized, with audio workers
// stopped before disposal, as required by the original pool's lifetime rules.
// A trivial intrusive registry avoids changing the cached native pool ABI and
// has no CRT destructor that could run before longer-lived static audio pools.
inline AudioPoolHeapBinding*& audioPoolHeapBindings() {
    static AudioPoolHeapBinding* head=nullptr;
    return head;
}
class AudioPoolHeapBinding final : public JKRDisposer {
public:
    JASGenericMemPool* pool;
    AudioPoolHeapBinding* next;
    explicit AudioPoolHeapBinding(JASGenericMemPool* value)
        :pool(value),next(audioPoolHeapBindings()) {
        audioPoolHeapBindings()=this;
    }
    ~AudioPoolHeapBinding() override {
        auto** link=&audioPoolHeapBindings();
        while(*link&&*link!=this)link=&(*link)->next;
        if(*link)*link=next;
        if(pool) {
            // Called by the actual owning heap before its backing is released.
            // Never inspect a free-list node after that release.
            pool->_0=nullptr;
            pool->mFreeMemCount=pool->mTotalMemCount=pool->mUsedMemCount=0;
            if(JASDram==mHeap)JASDram=nullptr;
        }
    }
};
inline AudioPoolHeapBinding* findAudioPoolHeapBinding(JASGenericMemPool* pool) {
    for(auto* it=audioPoolHeapBindings();it;it=it->next)if(it->pool==pool)return it;
    return nullptr;
}
inline void retainAudioPoolHeap(JASGenericMemPool* pool,JKRSolidHeap* heap) {
    if(!heap)throw std::logic_error("Audio pool allocation requires a live JASDram heap");
    if(auto* owner=findAudioPoolHeapBinding(pool)) {
        if(owner->mHeap!=heap)throw std::logic_error("Audio pool cannot grow across live heaps");
        return;
    }
    new(heap,0) AudioPoolHeapBinding(pool);
}
inline void releaseAudioPoolHeap(JASGenericMemPool* pool) {
    auto* owner=findAudioPoolHeapBinding(pool);
    if(!owner)return; // Its heap may already have disposed the binding.
    auto* heap=owner->mHeap;
    owner->pool=nullptr; // Normal pool destruction must not invalidate JASDram.
    owner->~AudioPoolHeapBinding();
    JKRHeap::free(owner,heap);
}
}
