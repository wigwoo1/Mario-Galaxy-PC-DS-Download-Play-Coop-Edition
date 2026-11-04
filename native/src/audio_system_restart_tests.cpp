#include <Native/HeapPlacement.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Native/DspAddressSpace.hpp>
#include <Native/DspAudioRuntime.hpp>
#include <Native/DspMailbox.hpp>
#include <JSystem/JAudio2/JASAiCtrl.hpp>
#include <JSystem/JAudio2/JASAudioThread.hpp>
#include <JSystem/JAudio2/JASChannel.hpp>
#include <JSystem/JAudio2/JASDSPChannel.hpp>
#include <JSystem/JAudio2/JASDriverIF.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <revolution/ai.h>
#include <revolution/dsp.h>
#include "native_jkr_aram_port.hpp"
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>

namespace {
std::mutex mutex;
std::condition_variable changed;
unsigned subframes=0,dacFrames=0,starts=0,stops=0,exactSamples=0;
unsigned referenceChunks=0;
bool requestStart=true;
OSThread* mainThread=nullptr;
JASDSPChannel* owner=nullptr;
std::array<s16,1120> expectedPcm{};
std::uintptr_t retiredBase=0;
LONG WINAPI recordAccessViolation(EXCEPTION_POINTERS* fault) {
    if(fault->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        const auto address=std::uintptr_t(fault->ExceptionRecord->ExceptionInformation[1]);
        std::printf("Restart access violation: operation=%llu address=%p instruction=%p retiredHeap=%p inRetiredHeap=%u lastRspMadep=%p\n",
            static_cast<unsigned long long>(fault->ExceptionRecord->ExceptionInformation[0]),
            reinterpret_cast<void*>(address),fault->ExceptionRecord->ExceptionAddress,
            reinterpret_cast<void*>(retiredBase),unsigned(retiredBase&&address>=retiredBase&&address-retiredBase<0x400000),JASDriver::lastRspMadep);
    }
    return EXCEPTION_CONTINUE_SEARCH; // Observe only; never suppress/recover.
}
s32 soundEvent(u32 event,JASDsp::TChannel* voice,void*) {
    assert(OSGetCurrentThread()!=mainThread);
    if(event==JASDSPChannel::CB_START) {
        JASWaveInfo info;info.mFormat=3;info.mLoopFlags=0;info.mSampleCount=200;
        info.mSampleLoopStart=0;info.mSampleLoopEnd=200;info.mpLastSample=info.mpPenultSample=0;
        voice->setWaveInfo(info,0x4000,0);voice->setPitch(0x1000);voice->setFilterMode(0);
        voice->setBusConnect(0,1);voice->setBusConnect(1,2);
        voice->setMixerInitVolume(0,0x7fff);voice->setMixerInitVolume(1,0x7fff);
        std::lock_guard<std::mutex> lock(mutex);++starts;
    } else if(event==JASDSPChannel::CB_STOP) {
        assert(!voice&&owner);owner->free();owner=nullptr;
        std::lock_guard<std::mutex> lock(mutex);++stops;changed.notify_all();return -1;
    } else if(event!=JASDSPChannel::CB_PLAY) { assert(false); }
    return 0;
}
s32 sync(void*) {
    assert(OSGetCurrentThread()!=mainThread);
    if(requestStart) {
        requestStart=false;
        owner=JASDSPChannel::alloc(20,soundEvent,nullptr);assert(owner);owner->start();
    }
    std::lock_guard<std::mutex> lock(mutex);++subframes;changed.notify_all();return 0;
}
void pcm(s16* samples,u32 frames) {
    assert(OSGetCurrentThread()!=mainThread);
    assert(frames==560);
    unsigned exact=0;bool hasSignal=false;
    for(unsigned i=0;i<frames;++i) {
        assert(samples[2*i]==samples[2*i+1]);
        hasSignal|=samples[2*i]!=0;
        exact+=samples[2*i]==5001;
    }
    if(hasSignal)for(unsigned i=0;i<frames*2;++i) {
        if(samples[i]!=expectedPcm[i])std::printf("PCM mismatch frame=%u sample=%u actual=%d expected=%d\n",dacFrames,i,int(samples[i]),int(expectedPcm[i]));
        assert(samples[i]==expectedPcm[i]);
    }
    std::lock_guard<std::mutex> lock(mutex);++dacFrames;exactSamples+=exact;
    if(hasSignal)++referenceChunks;changed.notify_all();
}
}
int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);mainThread=OSGetCurrentThread();
    auto* exceptionObserver=AddVectoredExceptionHandler(1,recordAccessViolation);assert(exceptionObserver);
    std::array<void*,3> reservations{};
    for(unsigned generation=0;generation<3;++generation) {
    auto* memory=VirtualAlloc(nullptr,0x400000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(memory);
    reservations[generation]=memory;
    {
    {std::lock_guard<std::mutex> lock(mutex);subframes=dacFrames=starts=stops=exactSamples=referenceChunks=0;requestStart=true;owner=nullptr;}
    const auto base=reinterpret_cast<std::uintptr_t>(memory);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x400000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    const auto initial=root.getTotalFreeSize();
    JASDram=JKRSolidHeap::create(0x300000,&root,false);assert(JASDram);
    std::vector<u8> backing(0x10000);
    auto aram=std::make_shared<NativeAramMemory>(backing.data(),backing.size());
    assert(nativeBindAramTransport(aram));
    std::array<u8,416> source{};
    for(unsigned i=0;i<200;++i){source[2*i]=0x27;source[2*i+1]=0x10;}
    assert(nativeUploadAram(0x4000,source.data(),source.size()));
    Native::nativeDspAddressSpace().clear();
    // Independent scalar reference for a 200-sample PCM16 step, four initial
    // zero history samples, original phase-zero FIR coefficients, 1.15 bus
    // volume and unity output volume. Include both start and end transients.
    assert(JASDsp::DSPRES_FILTER[0]==0x0c3966ad&&JASDsp::DSPRES_FILTER[1]==0x0d46ffdf);
    const int coefficients[]={0x0c39,0x66ad,0x0d46,-33};
    for(unsigned i=0;i<560;++i) {
        int sum=0;
        for(unsigned j=0;j<4;++j)if(i+j>=4&&i+j<204)sum+=coefficients[j]*10000;
        const int filtered=sum>>15;
        const s16 sample=s16((filtered*32767)>>16);
        expectedPcm[2*i]=expectedPcm[2*i+1]=sample;
    }
    // Frame commands capture the mixer level before DSP-sync callbacks run.
    JASDsp::setDSPMixerLevel(.25f);
    assert(JASDriver::registerDspSyncCallback(sync,nullptr));JASDriver::dacCallbackFunc=pcm;
    std::printf("Starting complete original audio worker generation%u on fresh heap %p; previous DAC pointer=%p\n",generation+1,memory,JASDriver::lastRspMadep);
    JASAudioThread::create(16);
    auto* audio=JASAudioThread::getThreadPointer();assert(audio);
    assert(nativeDspAudioRuntimeActive());
    assert(Native::nativeDspAddressSpace().resolve(AIGetDMAStartAddr(),AIGetDMALength()));
    auto* liveChannels=JASDsp::CH_BUF;
    JASDsp::boot(JASAudioThread::DSPCallback);
    assert(nativeDspAudioRuntimeActive()&&JASDsp::CH_BUF==liveChannels);
    std::puts("PASS original create/startup handshake, actual detached JKR worker, DSP boot and mapped native AI initialization");
    {
        std::unique_lock<std::mutex> lock(mutex);
        const bool ready=changed.wait_for(lock,std::chrono::seconds(8),[]{return stops==1&&exactSamples>=100&&referenceChunks>=1&&dacFrames>=30&&subframes>=100;});
        std::printf("thread counters: subframes=%u dac=%u starts=%u stops=%u exact5001=%u independentReferenceChunks=%u\n",subframes,dacFrames,starts,stops,exactSamples,referenceChunks);
        if(!ready){std::puts("FAIL sustained original audio loop deadline");std::_Exit(3);}
        assert(starts==1&&stops==1);
    }
    std::puts("PASS real AI-paced original thread loop, renderer PCM through original DAC interleave, natural voice EOF and sustained DMA/DSP messages");
    audio->stop();
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(!OSIsThreadTerminated(audio->getThreadRecord())&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
    assert(OSIsThreadTerminated(audio->getThreadRecord())&&!nativeDspAudioRuntimeActive());
    unsigned stoppedCount;
    {std::lock_guard<std::mutex> lock(mutex);stoppedCount=dacFrames;}
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    {std::lock_guard<std::mutex> lock(mutex);assert(dacFrames==stoppedCount);}
    assert(!owner&&JASDSPChannel::getNumFree()==64&&JASChannel::memPool_.getFreeMemCount()==72);
    JASDriver::rejectCallback(sync,nullptr);JASDriver::dacCallbackFunc=nullptr;AIRegisterDMACallback(nullptr);
    audio->~JASAudioThread();JKRHeap::free(audio,JASDram);
    assert(!JASAudioThread::getThreadPointer()&&JKRThread::sThreadList.getNumLinks()==0&&root.check());
    Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();
    std::puts("PASS original STOP exits real worker, joins AI worker, unregisters renderer and prevents late DAC calls; all64 DSP/72 parent slots free");
    JKRHeap::destroy(JASDram);
    assert(!JASDram&&JASChannel::memPool_.getTotalMemCount()==0&&root.check()&&root.getTotalFreeSize()==initial);
    }
    // Decommit physical backing and preserve only an inaccessible reservation
    // so the next generation cannot hide stale pointers by reusing its address.
    assert(VirtualFree(memory,0x400000,MEM_DECOMMIT));
    retiredBase=reinterpret_cast<std::uintptr_t>(memory);
    std::printf("PASS audio generation%u: real worker/device stopped, heap reclaimed and backing decommitted\n",generation+1);
    }
    for(auto* reservation:reservations)assert(VirtualFree(reservation,0,MEM_RELEASE));
    nativeSetDspAudioBootHooks(nullptr,nullptr);DSPInit();
    assert(RemoveVectoredExceptionHandler(exceptionObserver));
    std::puts("PASS three complete original audio-worker/DSP/AI lifetimes with released physical heap backing, independent exact PCM references and normal main return");
}
