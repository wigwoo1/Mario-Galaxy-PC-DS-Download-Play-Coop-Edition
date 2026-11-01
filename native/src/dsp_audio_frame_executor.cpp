// SPDX-License-Identifier: GPL-2.0-or-later
// SMG sequencing adapted from Dolphin ZeldaUCode::RenderAudio.
// Copyright 2008 Dolphin Emulator Project. See third_party/dolphin-zelda.
#include <Native/DspAudioFrameExecutor.hpp>
#include <stdexcept>
#include <utility>

namespace Native {
namespace {
constexpr u32 sync=0xDCD10004u, frameEnd=0xDCD10005u;
using Executor=DspAudioFrameExecutor;
Executor::Reply reject(Executor::Result result){Executor::Reply reply;reply.result=result;return reply;}
Executor::Reply acknowledge(u16 token){
    Executor::Reply reply;reply.result=Executor::Result::Applied;
    reply.mails={sync,0xF3550000u|token,0};reply.count=2;return reply;
}
}
DspAudioFrameExecutor::Reply DspAudioFrameExecutor::processBatch(const u32* words,std::size_t count){
    DspAudioCommandHle::Command command;
    switch(DspAudioCommandHle::decodeBatch(words,count,command)){
    case DspAudioCommandHle::Validation::Malformed:return reject(Result::Malformed);
    case DspAudioCommandHle::Validation::Unsupported:return reject(Result::Unsupported);
    case DspAudioCommandHle::Validation::Valid:break;
    }
    if(state_==State::Faulted||state_==State::WaitingContinue)return reject(Result::WrongOrder);
    using Kind=DspAudioCommandHle::Kind;
    if(command.kind==Kind::ChannelGroup){
        if(state_!=State::Rendering||command.group!=group_)return reject(Result::WrongOrder);
        try{
            if(!group_)renderer_.PrepareFrame();
            for(unsigned voice=0;voice<16;++voice)
                if(command.activeChannels&(0x8000u>>voice))renderer_.AddVoice(group_*16+voice);
            if(++group_!=4)return reject(Result::Applied);
            // Upstream queues SYNC before FinalizeFrame, but its mail handler
            // defers delivery. Our synchronous consumer must not see success
            // until both PCM planes and post-render reverb have been stored.
            renderer_.FinalizeFrame();
            Reply reply=acknowledge(static_cast<u16>(0xFF00u|completed_));
            reply.result=Result::SubframeRendered;
            group_=0;
            if(++completed_==requested_){state_=State::WaitingContinue;reply.mails[2]=frameEnd;reply.count=3;}
            return reply;
        }catch(...){
            // A voice may already have advanced. Never retry the partially
            // executed subframe or manufacture its completion notification.
            state_=State::Faulted;return reject(Result::RenderFailed);
        }
    }
    if(state_!=State::Idle)return reject(Result::WrongOrder);
    try{
        if(command.kind==Kind::Varam){
            memory_.CheckRange(command.arguments[0],1);
            renderer_.SetARAMBaseAddr(command.arguments[0]);aram_=true;
            return acknowledge(command.completionToken);
        }
        if(command.kind==Kind::Tables){
            const auto& args=command.arguments;
            // Check all spans before changing any renderer configuration.
            memory_.CheckRange(args[0],64*0x180);
            memory_.CheckRange(args[1],0x500);
            memory_.CheckRange(args[2],0x40);
            memory_.CheckRange(args[3],4*0x20);
            std::array<s16,0x100> resampling{},patterns{};
            std::array<s16,0x80> sine{};std::array<s16,0x20> afc{};
            memory_.CopyFromEmuSwapped(resampling.data(),args[1],sizeof(resampling));
            memory_.CopyFromEmuSwapped(patterns.data(),args[1]+0x200,sizeof(patterns));
            memory_.CopyFromEmuSwapped(sine.data(),args[1]+0x400,sizeof(sine));
            memory_.CopyFromEmuSwapped(afc.data(),args[2],sizeof(afc));
            renderer_.SetVPBBaseAddress(args[0]);renderer_.SetReverbPBBaseAddress(args[3]);
            renderer_.SetResamplingCoeffs(std::move(resampling));renderer_.SetConstPatterns(std::move(patterns));
            renderer_.SetSineTable(std::move(sine));renderer_.SetAfcCoeffs(std::move(afc));tables_=true;
            return acknowledge(command.completionToken);
        }
        if(!tables_||!aram_)return reject(Result::WrongOrder);
        if(!command.subframes)return reject(Result::Malformed);
        const std::size_t outputBytes=std::size_t(command.subframes)*0xA0;
        memory_.CheckRange(command.arguments[0],outputBytes);
        memory_.CheckRange(command.arguments[1],outputBytes);
        renderer_.SetOutputLeftBufferAddr(command.arguments[0]);
        renderer_.SetOutputRightBufferAddr(command.arguments[1]);
        renderer_.SetOutputVolume(command.mixerLevel);
        // SMG COMBINED_CMD_0D consumes two additional opaque arguments; as
        // in the pinned upstream handler, they do not configure output.
        requested_=command.subframes;completed_=group_=0;state_=State::Rendering;
        return reject(Result::Applied); // Frame acceptance is not completion.
    }catch(const std::out_of_range&){return reject(Result::InvalidMemory);}
}
bool DspAudioFrameExecutor::continueAfterFrame()noexcept{
    if(state_!=State::WaitingContinue)return false;
    state_=State::Idle;return true;
}
}
