// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <Native/DspAudioFrameExecutor.hpp>
#include <array>
#include <vector>
namespace Native {
// Single-owner bridge for the actual native JAS scalar/pointer layouts. Host
// mappings and the ARAM reader/context must remain alive throughout each call.
// It returns completion mails only after native voice/effects/PCM write-back;
// delivery to the live mailbox/audio thread remains the caller's responsibility.
class DspNativeAudioBridge {
public:
    using Reply=DspAudioFrameExecutor::Reply;
    using Result=DspAudioFrameExecutor::Result;
    DspNativeAudioBridge(const DspAddressSpace& host,ZeldaAudioMemory::SampleReader aram,void* context);
    Reply processBatch(const u32* words,std::size_t count);
    bool continueAfterFrame()noexcept{return !failed_&&executor_.continueAfterFrame();}
    bool failed()const noexcept{return failed_;}
    DspAudioFrameExecutor::State state()const noexcept{return executor_.state();}
private:
    struct Ring {std::vector<u8> bytes;u32 wire=0,host=0;std::size_t used=0;};
    const DspAddressSpace& host_;
    DspAddressSpace wire_;
    ZeldaAudioMemory memory_{wire_};
    DspAudioFrameExecutor executor_{memory_};
    std::array<u8,64*0x180> voices_{};
    std::array<u8,0x500> coefficients_{};
    std::array<u8,0x40> afc_{};
    std::array<u8,4*0x20> effects_{};
    std::array<u8,255*0xA0> left_{},right_{};
    std::array<Ring,4> rings_{};
    std::array<u32,4> hostTables_{};
    std::array<u32,2> hostOutput_{};
    std::array<u32,4> wireTables_{};
    u32 wireLeft_=0,wireRight_=0,wireAram_=0;
    u8 aramTag_=0;
    bool failed_=false;
    void* hostSpan(u32 address,std::size_t bytes)const;
    void refreshEffects();
    void commitOutput(unsigned subframe);
};
}
