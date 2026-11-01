// SPDX-License-Identifier: GPL-2.0-or-later
// SMG sequencing adapted from Dolphin ZeldaUCode::RenderAudio.
// Copyright 2008 Dolphin Emulator Project. See third_party/dolphin-zelda.
#pragma once
#include <Native/DspAudioCommandHle.hpp>
#include <Native/ZeldaAudioRenderer.hpp>

namespace Native {
// Executes decoded SMG commands against canonical big-endian DSP memory.
// The native JAS host-layout bridge and mailbox delivery are separate adapters.
// Single-owner audio-thread object: never call concurrently or reentrantly.
class DspAudioFrameExecutor {
public:
    enum class State { Idle, Rendering, WaitingContinue, Faulted };
    enum class Result { Applied, SubframeRendered, Malformed, Unsupported,
                        WrongOrder, InvalidMemory, RenderFailed };
    struct Reply {
        Result result=Result::Malformed;
        // DSP_SYNC + token, optionally followed by DSP_FRAME_END. The caller
        // may deliver these only after this method returns successfully.
        std::array<u32,3> mails{};
        unsigned count=0;
    };
    explicit DspAudioFrameExecutor(ZeldaAudioMemory& memory):memory_(memory),renderer_(memory){}
    Reply processBatch(const u32* words,std::size_t count);
    // The original DSP dispatcher sends MAIL_CONTINUE after DSP_FRAME_END.
    // This gate must be released explicitly before accepting another frame.
    bool continueAfterFrame() noexcept;
    State state()const noexcept{return state_;}
    unsigned completedSubframes()const noexcept{return completed_;}
    unsigned nextGroup()const noexcept{return group_;}
private:
    ZeldaAudioMemory& memory_;
    ZeldaAudioRenderer renderer_;
    State state_=State::Idle;
    bool tables_=false,aram_=false;
    unsigned requested_=0,completed_=0,group_=0;
};
}
