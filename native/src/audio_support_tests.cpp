#include "Game/AudioLib/AudUtil.hpp"
#include <JSystem/JAudio2/JAUAudience.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(float left,float right){return std::fabs(left-right)<0.00001f;}

int main(){
 assert(near(AudUtil::linerMax(-1,100,2.0f),0.0f));
 assert(near(AudUtil::linerMax(0,100,2.0f),0.0f));
 assert(near(AudUtil::linerMax(50,100,2.0f),1.0f));
 assert(near(AudUtil::linerMax(100,100,2.0f),2.0f));
 assert(near(AudUtil::linerMax(200,100,2.0f),2.0f));
 assert(near(AudUtil::linerMinMax(-1,0,100,0.25f,1.25f),0.25f));
 assert(near(AudUtil::linerMinMax(0,0,100,0.25f,1.25f),0.25f));
 assert(near(AudUtil::linerMinMax(50,0,100,0.25f,1.25f),0.75f));
 assert(near(AudUtil::linerMinMax(100,0,100,0.25f,1.25f),1.25f));
 assert(near(AudUtil::linerMinMax(101,0,100,0.25f,1.25f),1.25f));
 assert(near(AudUtil::linerMinMax(10,10,10,4.0f,9.0f),4.0f));

 JAUAudience3DSetting audience;
 assert(near(audience.mFarDistance,7000.0f)&&near(audience.mNearDistance,800.0f));
 assert(near(audience.mMinVolume,0.005f)&&near(audience.mBehindDolbyZ,8000.0f));
 assert(near(audience.mFrontDolbyZ,-4000.0f)&&near(audience.mMinFxMix,0.0f));
 assert(near(audience.mMaxFxMix,0.1f)&&near(audience.mMaxDolby,1.0f));
 assert(near(audience.mCenterDolby,0.8f)&&near(audience.mMinDolby,0.0f));
 assert(near(audience.mPanAmplitude,0.5f)&&audience.mPriorityDownRange==255);
 assert(near(audience.mSonicSpeed,3000.0f)&&near(audience.mPitchDeltaRatioRange,1.5f));
 audience.mFarDistance=1.0f;
 audience.init();
 assert(near(audience.mFarDistance,7000.0f));
 std::puts("PASS original audio interpolation and 3D audience defaults");
}
