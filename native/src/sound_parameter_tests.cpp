#include <JSystem/JAudio2/JAISoundParams.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
 JAISoundParamsMove state;state.init();
 JASSoundParams a,b,out;a.init();b.init();a.mVolume=.5f;b.mVolume=.25f;a.mPitch=2;b.mPitch=3;a.mPan=.25f;b.mPan=.75f;a.mFxMix=.125f;b.mFxMix=.25f;a.mDolby=.25f;b.mDolby=.5f;
 out.combine(a,b);assert(out.mVolume==.125f&&out.mPitch==6&&out.mPan==.5f&&out.mFxMix==.375f&&out.mDolby==.75f);
 a.combine(a,b);assert(a.mVolume==out.mVolume&&a.mPitch==out.mPitch&&a.mPan==out.mPan&&a.mFxMix==out.mFxMix&&a.mDolby==out.mDolby);
 state.moveVolume(0,4);state.movePitch(2,4);state.movePan(1,4);state.moveFxMix(1,4);state.moveDolby(1,4);
 for(unsigned i=1;i<=4;++i){state.calc();assert(std::fabs(state.mParams.mVolume-(1-i*.25f))<1e-6f);assert(std::fabs(state.mParams.mPitch-(1+i*.25f))<1e-6f);}
 assert(state.mParams.mPan==1&&state.mParams.mFxMix==1&&state.mParams.mDolby==1);
 state.calc();assert(state.mParams.mVolume==0&&state.mParams.mPitch==2);
 state.moveVolume(1,8);state.calc();state.moveVolume(.75f,0);state.calc();assert(state.mParams.mVolume==.75f&&state.mTransition.mVolume.mRemainingSteps==0);
 state.movePitch(.5f,1);state.calc();assert(state.mParams.mPitch==.5f);
 std::puts("PASS original sound parameter transitions: exact target, idle, immediate cancellation and one-step update");
}
