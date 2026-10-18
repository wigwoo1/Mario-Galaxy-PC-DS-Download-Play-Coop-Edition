#pragma once
#include "animation.hpp"
#include "frame_controller.hpp"
// Mirrors J3DFrameCtrl::update. One tick is one animation update, not a display refresh.
struct AnimationClock {
    uint8_t mode=2,state=0;float start=0,end=0,loop=0,rate=1,frame=0;
    explicit AnimationClock(const JointAnimation& animation):mode(animation.loop),end(animation.duration){}
    void tick(){
        assetCheck(mode<=4&&std::isfinite(frame)&&std::isfinite(rate)&&std::abs(rate)<=4096,"Invalid native frame controller rate/state");
        auto integral=[](float value){return std::isfinite(value)&&value>=-32768&&value<=32767&&std::floor(value)==value;};
        assetCheck(integral(start)&&integral(end)&&integral(loop)&&end>start&&loop>=start&&loop<end,"Invalid native frame controller range");
        assetCheck(frame>=start-4096&&frame<=end+4096,"Native frame controller frame outside bounded range");
        J3DFrameCtrl controller;controller.mAttribute=mode;controller.mState=state;controller.mStart=s16(start);controller.mEnd=s16(end);controller.mLoop=s16(loop);controller.mRate=rate;controller.mFrame=frame;controller.update();
        state=controller.mState;rate=controller.mRate;frame=controller.mFrame;
    }
    void referenceTick(){
        assetCheck(mode<=4&&std::isfinite(frame)&&std::isfinite(rate)&&std::isfinite(start)&&std::isfinite(end)&&std::isfinite(loop),"Invalid animation clock");
        assetCheck(end>start&&loop>=start&&loop<end,"Empty animation clock range");
        state=0;frame+=rate;assetCheck(std::isfinite(frame),"Animation clock overflow");
        switch(mode){
        case 0:case 1:
            if(frame<start){frame=start;rate=0;state|=1;}
            if(frame>=end){frame=mode==0?end-.001f:start;rate=0;state|=1;}
            break;
        case 2:
            // Bound work for large jumps; the original uses repeated additions/subtractions.
            if(frame<start){state|=2;float span=loop-start;if(span>0)frame+=std::ceil((start-frame)/span)*span;}
            if(frame>=end){state|=2;float span=end-loop;frame-=std::floor((frame-end)/span+1)*span;}
            break;
        case 3:
            if(frame>=end){frame=end-(frame-end);rate=-rate;}
            if(frame<start){frame=start-(frame-start);rate=0;state|=1;}
            break;
        case 4:
            if(frame>=end-1){frame=end-1-(frame-(end-1));rate=-rate;}
            if(frame<start){frame=start-(frame-start);rate=-rate;state|=2;}
            break;
        }
    }
};
// Fixed-step accumulation keeps animation speed independent of rendering cadence.
struct AnimationTicker {
    double remainder=0;
    size_t advance(AnimationClock& clock,double seconds){
        assetCheck(std::isfinite(seconds)&&seconds>=0&&seconds<=10,"Invalid animation elapsed time");
        remainder+=seconds*60;size_t count=size_t(std::floor(remainder+1e-9));remainder-=count;
        if(remainder<0)remainder=0;for(size_t i=0;i<count;++i)clock.tick();return count;
    }
};
inline void animationClockTests(){
    auto check=[](bool value){assetCheck(value,"Animation clock regression failed");};
    for(uint8_t mode=0;mode<5;++mode)for(float rate:{-3.f,-1.f,.5f,1.f,3.f}){
        JointAnimation sample{mode,0,12,{}};AnimationClock original(sample),reference(sample);original.rate=reference.rate=rate;original.loop=reference.loop=4;original.frame=reference.frame=6;
        for(int i=0;i<100;++i){original.tick();reference.referenceTick();check(std::abs(original.frame-reference.frame)<.0001&&original.rate==reference.rate&&original.state==reference.state);}
    }
    JointAnimation animation{0,0,4,{}};AnimationClock once(animation);for(int i=0;i<4;++i)once.tick();check(once.state==1&&once.rate==0&&std::abs(once.frame-3.999f)<.0001);
    animation.loop=1;AnimationClock reset(animation);reset.frame=3;reset.tick();check(reset.frame==0&&reset.rate==0&&reset.state==1);
    animation.loop=2;AnimationClock loop(animation);loop.frame=3;loop.tick();check(loop.frame==0&&loop.state==2);loop.rate=9;loop.tick();check(loop.frame==1&&loop.state==2);
    loop.loop=2;loop.frame=0;loop.rate=-1;loop.tick();check(loop.frame==1&&loop.state==2);
    animation.loop=3;AnimationClock reverse(animation);reverse.frame=3;reverse.tick();check(reverse.frame==4&&reverse.rate==-1);for(int i=0;i<5;++i)reverse.tick();check(reverse.frame==1&&reverse.rate==0&&reverse.state==1);
    animation.loop=4;AnimationClock pingpong(animation);pingpong.frame=2;pingpong.tick();check(pingpong.frame==3&&pingpong.rate==-1);for(int i=0;i<4;++i)pingpong.tick();check(pingpong.frame==1&&pingpong.rate==1&&pingpong.state==2);
    animation.loop=2;animation.duration=60;AnimationClock a(animation),b(animation);AnimationTicker ta,tb;
    for(int i=0;i<120;++i)ta.advance(a,1.0/120);for(int i=0;i<30;++i)tb.advance(b,1.0/30);check(a.frame==b.frame&&a.frame==0);
    bool rejected=false;try{ta.advance(a,-1);}catch(const std::exception&){rejected=true;}check(rejected);
}
