// SPDX-License-Identifier: GPL-2.0-or-later
#include <Native/DspAudioDispatch.hpp>
#include <Native/DspMailbox.hpp>
#include <revolution/dsp.h>
#include <revolution/os.h>
void Dsp_Update_Request();
namespace Native {
s32 DspAudioDispatch::process(const u32* words,u32 count,u16* completion)noexcept{
    if(!completion||failed_||!request_)return -3;
    try{
        // A full SMG request has at most255 subframes. A bounded queue holds
        // their notifications plus the original setup task queue's15 entries.
        if(mails_.size()>1020)return -3;
        const auto reply=bridge_.processBatch(words,count);
        using Result=DspNativeAudioBridge::Result;
        if(reply.result!=Result::Applied&&reply.result!=Result::SubframeRendered){failed_=bridge_.failed();return -3;}
        for(unsigned i=0;i<reply.count;++i)mails_.push_back(reply.mails[i]);
        *completion=0; // DSPSend must not complete a real task synchronously.
        return 2;
    }catch(...){failed_=true;return -3;}
}
void DspAudioDispatch::service()noexcept{
    if(servicing_||failed_||mails_.empty())return;
    const BOOL enabled=OSDisableInterrupts();
    if(!enabled){OSRestoreInterrupts(enabled);return;} // Outer critical section still owns delivery.
    servicing_=true;
    try{
        auto& mailbox=nativeDspMailbox();
        while(!mails_.empty()){
            const u32 prefix=mails_.front();
            if(mailbox.fromDspFull()||!mailbox.tryWriteFromDsp(prefix))throw 1;
            if(DSPReadMailFromDSP()!=prefix)throw 1;
            mails_.pop_front();
            if(prefix==0xDCD10004){
                if(mails_.empty()||(mails_.front()>>16)!=0xF355)throw 1;
                const u32 token=mails_.front();mails_.pop_front();
                if(!mailbox.tryWriteFromDsp(token))throw 1;
                request_(nullptr);
                if(mailbox.fromDspFull())throw 1; // Callback must consume its own token.
                if((token&0xFF00)==0xFF00)++subframes_;
            }else if(prefix==0xDCD10005){
                if(!bridge_.continueAfterFrame())throw 1;
                ++frames_;
                // Original audio-only handler resumes the pending frame after
                // FRAME_END. Reentrant service calls defer to this outer drain.
                Dsp_Update_Request();
            }else throw 1;
        }
    }catch(...){failed_=true;}
    servicing_=false;OSRestoreInterrupts(enabled);
}
}
