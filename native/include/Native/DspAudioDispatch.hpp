// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <Native/DspNativeAudioBridge.hpp>
#include <deque>
namespace Native {
// Cooperative audio-only equivalent of the SYNC/FRAME_END task-dispatch cases.
// The original JAudio request callback consumes F355 mail using SDK functions.
// Owner serializes commands/service/teardown on the native audio thread. Call
// service at an interrupt-enabled boundary before that thread blocks for mail.
class DspAudioDispatch {
public:
    using Request=void(*)(void*);
    DspAudioDispatch(DspNativeAudioBridge& bridge,Request request):bridge_(bridge),request_(request){}
    s32 process(const u32* words,u32 count,u16* completion)noexcept;
    void service()noexcept;
    bool failed()const noexcept{return failed_;}
    std::size_t pending()const noexcept{return mails_.size();}
    unsigned subframeNotifications()const noexcept{return subframes_;}
    unsigned frameEnds()const noexcept{return frames_;}
private:
    DspNativeAudioBridge& bridge_;
    Request request_;
    std::deque<u32> mails_;
    bool servicing_=false,failed_=false;
    unsigned subframes_=0,frames_=0;
};
}
