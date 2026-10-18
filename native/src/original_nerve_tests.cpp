#include "Game/LiveActor/Nerve.hpp"
#include <cassert>
#include <cmath>
#include "Game/System/NerveExecutor.hpp"
#include "Game/Util/NerveUtil.hpp"
#include <cstdio>
struct State final : Nerve {
 mutable int calls=0, ends=0, observed=-99;
 const Nerve* next=nullptr;
 void execute(Spine* s) const override { ++calls; observed=s->mStep; if(next)s->setNerve(next); }
 void executeOnEnd(Spine*) const override { ++ends; }
};
int main(){
 State a,b,c; Spine s(nullptr,&a);
 s.update(); assert(a.calls==1 && a.observed==0 && s.mStep==1);
 s.setNerve(&b); assert(a.ends==1 && s.mStep==-1 && s.getCurrentNerve()==&b);
 s.setNerve(&c); assert(a.ends==1 && b.ends==0);
 s.update(); assert(c.calls==1 && c.observed==0 && s.mStep==1 && b.calls==0);
 c.next=&a; s.update(); assert(c.ends==1 && s.getCurrentNerve()==&a && s.mStep==0);
 s.update(); assert(a.calls==2 && a.observed==0 && s.mStep==1);
 NerveExecutor executor("rate test"); executor.initNerve(&b);
 assert(MR::isFirstStep(&executor));
 assert(MR::calcNerveRate(&executor,0)==1.0f);
 for(int i=0;i<10;++i)executor.updateNerve();
 assert(MR::calcNerveRate(&executor,20)==0.5f);
 assert(MR::calcNerveValue(&executor,20,10.0f,30.0f)==20.0f);
 assert(std::fabs(MR::calcNerveEaseInRate(&executor,20)-0.2928932f)<0.002f);
 assert(std::fabs(MR::calcNerveEaseOutRate(&executor,20)-0.7071068f)<0.002f);
 assert(std::fabs(MR::calcNerveEaseInOutValue(&executor,20,10.0f,30.0f)-20.0f)<0.002f);
 assert(std::fabs(MR::calcNerveEaseInOutValue(&executor,5,15,10.0f,30.0f)-20.0f)<0.002f);
 executor.setNerve(&a); assert(MR::isNewNerve(&executor));
 assert(MR::calcNerveRate(&executor,20)==0.0f);
 executor.updateNerve(); assert(executor.getNerveStep()==1);
 std::puts("Original Spine/Nerve transitions PASS: deferred replacement, end callbacks, in-update transition and first-step reset");
}

