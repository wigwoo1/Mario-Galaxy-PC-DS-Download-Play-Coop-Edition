#include <Native/DspAddressSpace.hpp>
#include <Native/DspMailbox.hpp>
#include <Native/DspUcodeImage.hpp>
#include <JSystem/JAudio2/JASDSPInterface.hpp>
#include <JSystem/JAudio2/dspproc.hpp>
#include <JSystem/JAudio2/dsptask.hpp>
#include <revolution/dsp.h>
#include <revolution/os.h>
#include <cassert>
#include <cstdio>
#include <cstring>

void Dsp_Running_Start();
void DspInitWork();
static unsigned rejected=0;
extern "C" BOOL OSDisableInterrupts(){return 0;}
extern "C" BOOL OSRestoreInterrupts(BOOL enabled){return enabled;}
extern "C" void OSReport(const char* format,...){
    if(std::strstr(format,"Native DSP HLE rejected command batch"))++rejected;
}
static void boot(){
    DSPInit();DspInitWork();
    const auto* image=Native::originalDspUcodeImage();
    const auto size=Native::originalDspUcodeImageBytes();
    assert(nativeDspBeginRomTask(image,size));
    const auto guest=Native::nativeDspAddressSpace().addressOf(image,size);assert(guest);
    assert(DSPReadMailFromDSP()==0x8071FEEDu);
    const u32 upload[]={0x80F3A001u,guest,0x80F3C002u,0,0x80F3A002u,0x1F00u,0x80F3B002u,0,0x80F3D001u,0};
    for(u32 word:upload){DSPSendMailToDSP(word);assert(!DSPCheckMailToDSP());}
    assert(DSPReadMailFromDSP()==0xDCD10000u);
    assert(DSPReadMailFromDSP()==0xF3551111u);
    Dsp_Running_Start();
}
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);boot();
    JASDsp::TChannel channels[64]{};JASDsp::CH_BUF=channels;
    // Exercise the real 64-channel map and original command generator, rather
    // than the old recording test's arbitrary group 0x55 and stubbed map.
    for(unsigned voice=0;voice<64;++voice){
        channels[voice].mIsActive=1;
        for(unsigned group=0;group<4;++group)
            assert(DSP_CreateMap2(group)==(group==voice/16?(0x8000u>>(voice%16)):0));
        channels[voice].mIsActive=0;
    }
    for(unsigned voice:{0u,15u,16u,31u,33u,62u})channels[voice].mIsActive=1;
    const u16 expected[]={0x8001,0x8001,0x4000,0x0002};
    DsetVARAM(0x12345678);DsetupTable(64,0x1000,0x2000,0x3000,0x4000);
    DsyncFrame2ch(7,0x5000,0x6000);
    for(unsigned group=0;group<4;++group){
        assert(DSP_CreateMap2(group)==expected[group]);
        const unsigned before=rejected;
        DSPReleaseHalt2(group);
        std::printf("original group=%u activeMap=%04X rejected=%u\n",group,unsigned(expected[group]),rejected-before);
    }
    assert(!DSPCheckMailFromDSP());
    // This remains the explicitly silent fallback. Accepting valid packet
    // shapes must not invent rendered subframes or channel-completion mail.
    if(rejected){std::printf("REPRODUCED %u production channel-group packets rejected\n",rejected);return 2;}
    std::puts("PASS original 64-channel maps and four group packets through booted host transport; no fabricated render completion");
}
