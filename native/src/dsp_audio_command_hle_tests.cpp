#include <Native/DspAudioCommandHle.hpp>

#include <cassert>
#include <cstdint>
#include <cstdio>

int main() {
    Native::DspAudioCommandHle hle;
    std::uint16_t token = 0;

    const std::uint32_t varam[] = {0x8E000000u, 0x12345678u};
    assert(hle.processBatch(varam, 2, &token) ==
           Native::DspAudioCommandHle::Result::AcceptedSilent);
    assert(token == 0x8E00u);

    const std::uint32_t table[] = {
        0x81000040u, 0x1000u, 0x2000u, 0x3000u, 0x4000u
    };
    assert(hle.processBatch(table, 5, &token) ==
           Native::DspAudioCommandHle::Result::AcceptedSilent);
    assert(token == 0x8100u);

    const std::uint32_t frame[] = {
        0x82072000u, 0x12345678u, 0x87654321u, 0u, 0u
    };
    assert(hle.processBatch(frame, 5, &token) ==
           Native::DspAudioCommandHle::Result::AcceptedSilent);
    assert(token == 0x8207u);

    using Hle=Native::DspAudioCommandHle;
    Hle::Command decoded;
    assert(Hle::decodeBatch(varam,2,decoded)==Hle::Validation::Valid);
    assert(decoded.kind==Hle::Kind::Varam&&decoded.arguments[0]==varam[1]);
    assert(Hle::decodeBatch(table,5,decoded)==Hle::Validation::Valid);
    assert(decoded.kind==Hle::Kind::Tables&&decoded.arguments[3]==0x4000);
    assert(Hle::decodeBatch(frame,5,decoded)==Hle::Validation::Valid);
    assert(decoded.kind==Hle::Kind::Frame&&decoded.subframes==7&&decoded.mixerLevel==0x2000);
    assert(decoded.arguments[0]==frame[1]&&decoded.arguments[1]==frame[2]&&!decoded.arguments[2]&&!decoded.arguments[3]);
    for(std::uint32_t group=0;group<4;++group){
        for(std::uint32_t mask=0;mask<=0xFFFF;++mask){
            const std::uint32_t word=group<<16|mask;
            assert(Hle::decodeBatch(&word,0,decoded)==Hle::Validation::Valid);
            assert(decoded.kind==Hle::Kind::ChannelGroup&&decoded.group==group&&decoded.activeChannels==mask);
        }
        const std::uint32_t halt[]={group<<16|0x1234u};
        assert(hle.processBatch(halt,0,&token)==Hle::Result::AcceptedSilent&&token==group);
        assert(hle.processBatch(halt,1,&token)==Hle::Result::AcceptedSilent);
        const auto previous=decoded;
        assert(Hle::decodeBatch(halt,2,decoded)==Hle::Validation::Malformed);
        assert(decoded.kind==previous.kind&&decoded.activeChannels==previous.activeChannels&&decoded.group==previous.group);
    }
    for(std::uint32_t invalid:{0x00040000u,0x00551234u,0xFFFF1234u}){
        token=0xABCD;
        assert(hle.processBatch(&invalid,0,&token)==Hle::Result::Unsupported&&token==0xABCD);
    }

    assert(hle.processBatch(varam, 1, &token) ==
           Native::DspAudioCommandHle::Result::Malformed);
    const std::uint32_t badFrame[] = {0x82072000u, 1u};
    assert(hle.processBatch(badFrame, 2, &token) ==
           Native::DspAudioCommandHle::Result::Malformed);
    const std::uint32_t unsupported[] = {0x8D000000u};
    assert(hle.processBatch(unsupported, 1, &token) ==
           Native::DspAudioCommandHle::Result::Unsupported);
    assert(hle.processBatch(nullptr, 1, &token) ==
           Native::DspAudioCommandHle::Result::Malformed);
    assert(hle.processBatch(varam, 2, nullptr) ==
           Native::DspAudioCommandHle::Result::Malformed);

    std::puts("PASS bounded DSP packet decoding, all 262144 group masks, transactional rejection and explicit silent fallback tokens; no PCM execution");
}
