#pragma once
#include "Game/Util/LiveActorUtil.hpp"
inline void actorTimingTests(){
 struct Next:Nerve {mutable int calls=0;void execute(Spine* spine)const override{assetCheck(MR::isFirstStep(static_cast<LiveActor*>(spine->mExecutor)),"Timed actor transition first step incorrect");++calls;}} next;
 struct Wait:Nerve {const Nerve& next;mutable int first=0,interval=0;explicit Wait(const Nerve& n):next(n){}void execute(Spine* spine)const override{auto actor=static_cast<LiveActor*>(spine->mExecutor);if(MR::isFirstStep(actor))++first;if(MR::isIntervalStep(actor,2))++interval;auto step=actor->getNerveStep();assetCheck(MR::isStep(actor,step)&&MR::isLessEqualStep(actor,step)&&MR::isGreaterEqualStep(actor,step)&&!MR::isLessStep(actor,step)&&!MR::isGreaterStep(actor,step)&&!MR::isNewNerve(actor),"Actor timing predicate boundary failed");MR::setNerveAtStep(actor,&next,3);}} wait(next);
 NativeScene scene;auto& actor=scene.create<NativeScheduledSensorActor>("timed actor",wait);for(int i=0;i<4;++i)scene.tick();assetCheck(wait.first==1&&wait.interval==2&&actor.isNerve(&next)&&actor.getNerveStep()==0&&next.calls==0,"Timed actor transition occurred at wrong frame");scene.tick();assetCheck(next.calls==1,"Timed actor next state did not execute");actor.setNerve(&wait);assetCheck(MR::isNewNerve(&actor)&&MR::isLessStep(&actor,0)&&!MR::isFirstStep(&actor),"Pending actor nerve timing failed");
}
