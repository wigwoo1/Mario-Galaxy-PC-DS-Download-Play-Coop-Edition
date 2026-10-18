#include "frame_controller.hpp"
#include "assets.hpp"
#include "Game/Screen/MissLayout.hpp"
#include "Game/LiveActor/Nerve.hpp"
LayoutActor::LayoutActor(const char* name,bool):NameObj(name){}
LayoutActor::~LayoutActor()=default;
void LayoutActor::registerNativeAnimation(const std::string& name,u16 frames,bool loop){assetCheck(nativeDead&&!name.empty()&&frames>0&&frames<=32767,"Invalid native layout animation registration");assetCheck(animationTiming.emplace(name,std::make_pair(frames,loop)).second,"Duplicate native layout animation");}
void LayoutActor::startNativeAnimation(const char* name){nativeAnimation=name;nativeAnimStopped=false;auto timing=animationTiming.find(name);if(timing==animationTiming.end()){animationClock.reset();return;}animationClock=std::make_unique<J3DFrameCtrl>(s16(timing->second.first));animationClock->setAttribute(timing->second.second?2:0);animationClock->setFrame(0.0f);animationClock->setRate(1.0f);}
float LayoutActor::nativeAnimationFrame()const{return animationClock?animationClock->getFrame():0.0f;}
void LayoutActor::movement(){if(!nativeDead){assetCheck(bool(spine),"Missing layout nerve");spine->update();if(!nativeDead&&animationClock){animationClock->update();nativeAnimStopped=animationClock->checkState(1)||animationClock->getRate()==0.0f;}}}
void LayoutActor::appear(){nativeDead=false;}
void LayoutActor::kill(){nativeDead=true;nativeEffects.clear();}
void LayoutActor::initNerve(const Nerve* nerve){assetCheck(!spine&&nerve,"Invalid layout nerve initialization");spine=std::make_unique<Spine>(this,nerve);}
void LayoutActor::setNerve(const Nerve* nerve)const{assetCheck(bool(spine)&&nerve,"Invalid layout nerve change");spine->setNerve(nerve);}
bool LayoutActor::isNerve(const Nerve* nerve)const{return spine&&spine->getCurrentNerve()==nerve;}
s32 LayoutActor::getNerveStep()const{assetCheck(bool(spine),"Missing layout nerve step");return spine->mStep;}
void LayoutActor::initLayoutManager(const char* name,u32 layers){assetCheck(name&&layers==1,"Unsupported native layout layer count");nativeLayout=name;}
void LayoutActor::initEffectKeeper(int count,const char*,const void*){assetCheck(count==0,"Unsupported native pane effect keeper");}
namespace MR{
void connectToSceneLayout(LayoutActor*){}
void hideLayout(LayoutActor* actor){actor->nativeHidden=true;}
void showLayout(LayoutActor* actor){actor->nativeHidden=false;}
bool isFirstStep(const LayoutActor* actor){return actor->getNerveStep()==0;}
bool isStep(const LayoutActor* actor,s32 step){return actor->getNerveStep()==step;}
void startAnim(LayoutActor* actor,const char* name,u32 layer){assetCheck(layer==0&&name,"Invalid layout animation");actor->startNativeAnimation(name);}
J3DFrameCtrl* getAnimCtrl(LayoutActor* actor,u32 layer){assetCheck(layer==0&&actor->nativeAnimationController(),"Missing native layout animation controller");return actor->nativeAnimationController();}
void setAnimRate(LayoutActor* actor,f32 rate,u32 layer){assetCheck(std::isfinite(rate)&&rate>0&&rate<=4096,"Invalid native layout animation rate");getAnimCtrl(actor,layer)->setRate(rate);}
void setAnimFrameAndStop(LayoutActor* actor,f32 frame,u32 layer){auto ctrl=getAnimCtrl(actor,layer);assetCheck(std::isfinite(frame)&&frame>=0&&frame<=ctrl->getEnd(),"Invalid native layout animation frame");ctrl->setFrame(frame);ctrl->setRate(0);actor->nativeAnimStopped=true;}
bool isAnimStopped(const LayoutActor* actor,u32 layer){assetCheck(layer==0,"Invalid layout animation query");return actor->nativeAnimStopped;}
void emitEffect(LayoutActor* actor,const char* name){actor->nativeEffects.emplace_back(name);}
}
// Native compiler rejects weak symbols with anonymous-namespace linkage.
#undef ATTRIBUTE_WEAK
#define ATTRIBUTE_WEAK
namespace {
    NEW_NERVE(MissLayoutNrvWait, MissLayout, Wait);
    NEW_NERVE(MissLayoutNrvActive, MissLayout, Active);
    NEW_NERVE(MissLayoutNrvEnd, MissLayout, End);
};  // namespace

MissLayout::MissLayout() : LayoutActor("ミス!", true) {
}

void MissLayout::init(const JMapInfoIter& rIter) {
    MR::connectToSceneLayout(this);
    initLayoutManager("TooBad", 1);
    initEffectKeeper(0, nullptr, nullptr);
    initNerve(GET_NERVE_ANON(MissLayoutNrvWait));
    kill();
}

void MissLayout::kill() {
    LayoutActor::kill();
    setNerve(GET_NERVE_ANON(MissLayoutNrvWait));
}

bool MissLayout::isActive() const {
    return isNerve(GET_NERVE_ANON(MissLayoutNrvActive));
}

void MissLayout::start() {
    if (isNerve(GET_NERVE_ANON(MissLayoutNrvActive))) {
        return;
    }

    LayoutActor::appear();
    MR::hideLayout(this);
    setNerve(GET_NERVE_ANON(MissLayoutNrvActive));
}

void MissLayout::exeWait() {
}

void MissLayout::exeActive() {
    if (MR::isFirstStep(this)) {
        MR::showLayout(this);
        MR::startAnim(this, "Appear", 0);
    }

    if (MR::isStep(this, 18)) {
        MR::emitEffect(this, "TooBad");
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_ANON(MissLayoutNrvEnd));
    }
}

void MissLayout::exeEnd() {
}




