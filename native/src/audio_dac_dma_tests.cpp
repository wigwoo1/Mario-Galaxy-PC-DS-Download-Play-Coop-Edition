#include <Native/HeapPlacement.hpp>
#include <Native/DspAddressSpace.hpp>
#include <JSystem/JAudio2/JASAiCtrl.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <revolution/ai.h>
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <vector>

// This fixture supplies already-rendered planar buffers. It deliberately does
// not emulate frame completion or claim to run the original audio thread.
void JASDriver::finishDSPFrame() { assert(false && "Unexpected DSP scheduling in DAC transfer fixture"); }

int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    std::vector<u8> storage(0x100000+32);
    auto base=(reinterpret_cast<std::uintptr_t>(storage.data())+31)&~std::uintptr_t(31);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x100000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    const auto initial=root.getTotalFreeSize();
    JASDram=JKRSolidHeap::create(0x40000,&root,false);assert(JASDram);
    auto& space=Native::nativeDspAddressSpace();space.clear();
    JASDriver::initAI(nullptr);
    const auto bytes=JASDriver::getDacSize()*sizeof(s16);
    auto* initialBuffer=JASDriver::sDmaDacBuffer[2];
    assert(reinterpret_cast<std::uintptr_t>(initialBuffer)>UINT32_MAX);
    auto* initialResolved=space.resolve(AIGetDMAStartAddr(),AIGetDMALength());
    std::printf("initAI host=%p guest=%08X resolved=%p bytes=%u\n",initialBuffer,AIGetDMAStartAddr(),initialResolved,AIGetDMALength());
    assert(initialResolved==initialBuffer && AIGetDMALength()==bytes);
    std::puts("PASS original initAI submits its real 64-bit heap buffer through a bounded 32-bit DMA mapping");

    const auto samples=JASDriver::getFrameSamples();
    for(unsigned frame=0;frame<30;++frame) {
        auto* previous=JASDriver::lastRspMadep;
        JASDriver::sDspDacReadBuffer=0;JASDriver::sDspDacWriteBuffer=2;
        JASDriver::sDspStatus=1;
        auto* planar=JASDriver::sDspDacBuffer[1];
        for(unsigned i=0;i<samples;++i) {
            planar[i]=s16(-2000-int(i)-int(frame));
            planar[samples+i]=s16(1000+int(i)+int(frame));
        }
        JASDriver::updateDac();
        auto* mixed=JASDriver::sDmaDacBuffer[frame%3];
        assert(JASDriver::lastRspMadep==mixed);
        for(unsigned i=0;i<samples;++i) {
            assert(mixed[2*i]==s16(1000+int(i)+int(frame)));
            assert(mixed[2*i+1]==s16(-2000-int(i)-int(frame)));
        }
        auto* submitted=previous?previous:initialBuffer;
        assert(space.resolve(AIGetDMAStartAddr(),bytes)==submitted && AIGetDMALength()==bytes);
        assert(space.resolve(AIGetDMAStartAddr()+u32(bytes),1)==nullptr);
        assert(space.mappingCount()==(frame==0?1u:frame==1?2u:3u));
    }
    std::puts("PASS actual updateDac/readDspBuffer rotates all three buffers for30 updates, preserves exact stereo PCM and reuses bounded mappings");
    auto* blocked=JASDriver::sDmaDacBuffer[0];
    assert(space.unmap(space.addressOf(blocked,bytes)));
    assert(space.map(blocked,sizeof(s16)));
    const auto priorAddress=AIGetDMAStartAddr();
    JASDriver::lastRspMadep=blocked;
    bool rejected=false;
    try { JASDriver::updateDac(); } catch(const std::runtime_error&) { rejected=true; }
    assert(rejected && AIGetDMAStartAddr()==priorAddress);
    std::puts("PASS partial/overlapping buffer registration rejects the full DMA submission before changing the AI address");
    AIStopDMA();AIRegisterDMACallback(nullptr);space.clear();
    for(auto& p:JASDriver::sDmaDacBuffer)p=nullptr;
    JASDriver::sDspDacBuffer=nullptr;JASDriver::lastRspMadep=nullptr;
    JKRHeap::destroy(JASDram);JASDram=nullptr;
    assert(root.check() && root.getTotalFreeSize()==initial);
    std::puts("PASS DMA stopped before unmapping/freeing actual heap buffers; no device/thread/game execution claimed");
}
