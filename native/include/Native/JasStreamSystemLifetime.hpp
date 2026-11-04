#pragma once
#include <Native/HeapPlacement.hpp>
#include <JSystem/JAudio2/JASAramStream.hpp>
#include <JSystem/JAudio2/JASDriverIF.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JKernel/JKRDisposer.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <stdexcept>

extern bool UNK_BOOL_A;
extern bool UNK_BOOL_B;
namespace Native {
class JasStreamSystemOwner;
inline JasStreamSystemOwner*& jasStreamSystemOwner() {
    static JasStreamSystemOwner* owner=nullptr;
    return owner;
}
inline void clearJasStreamSystemReferences() noexcept {
    JASAramStream::sLoadThread=nullptr;
    JASAramStream::sReadBuffer=nullptr;
    JASAramStream::sBlockSize=JASAramStream::sChannelMax=0;
    UNK_BOOL_A=UNK_BOOL_B=false;
}
// Configuration/disposal are serialized. Stop the audio/DVD workers and return
// all streams before heap disposal; this does not cancel live stream jobs.
class JasStreamSystemOwner final : public JKRDisposer {
public:
    u8* buffer;
    JASTaskThread* worker;
    u32 blockSize,channelMax;
    JasStreamSystemOwner():buffer(JASAramStream::sReadBuffer),worker(JASAramStream::sLoadThread),
        blockSize(JASAramStream::sBlockSize),channelMax(JASAramStream::sChannelMax) {
        jasStreamSystemOwner()=this;
    }
    ~JasStreamSystemOwner() override {
        JASDriver::rejectCallback(JASAramStream::dvdErrorCheck,nullptr);
        clearJasStreamSystemReferences();
        jasStreamSystemOwner()=nullptr;
        if(JASDram==mHeap)JASDram=nullptr;
    }
};
inline bool reuseJasStreamSystem(JKRSolidHeap* heap,u32 block,u32 channels) {
    if(!heap)throw std::logic_error("Stream initialization requires a live audio heap");
    if(auto* owner=jasStreamSystemOwner()) {
        if(owner->mHeap!=heap||owner->blockSize!=block||owner->channelMax!=channels||
           owner->buffer!=JASAramStream::sReadBuffer||owner->worker!=JASAramStream::sLoadThread)
            throw std::logic_error("Dispose the current stream system before changing its configuration");
        return true;
    }
    // Do not adopt or erase an unrelated registration using the same function.
    JASCriticalSection critical;
    for(const auto& entry:JASDriver::sSubFrameCallback.mCallbacks)
        if(entry.mCallback==JASAramStream::dvdErrorCheck&&entry.mArg==nullptr)
            throw std::logic_error("Stream callback already registered without its lifetime owner");
    return false;
}
class JasStreamSystemInit {
    JKRSolidHeap* heap;
    bool registered=false;
public:
    explicit JasStreamSystemInit(JKRSolidHeap* value):heap(value){}
    JasStreamSystemInit(const JasStreamSystemInit&)=delete;
    JasStreamSystemInit& operator=(const JasStreamSystemInit&)=delete;
    bool registerCallback() {
        registered=JASDriver::registerSubFrameCallback(JASAramStream::dvdErrorCheck,nullptr);
        return registered;
    }
    void commit() {
        new(heap,0) JasStreamSystemOwner;
        registered=false;
    }
    ~JasStreamSystemInit() {
        if(!registered)return;
        JASDriver::rejectCallback(JASAramStream::dvdErrorCheck,nullptr);
        JKRHeap::free(JASAramStream::sReadBuffer,heap);
        clearJasStreamSystemReferences();
    }
};
}
