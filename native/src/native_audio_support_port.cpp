#include "Game/AudioLib/AudUtil.hpp"
#include <JSystem/JAudio2/JAUAudience.hpp>

namespace AudUtil {
f32 linerMax(s32 value,s32 maximum,f32 end){
 if(value<=0)return 0.0f;
 if(value>=maximum)return end;
 return end*value/maximum;
}

f32 linerMinMax(s32 value,s32 minimum,s32 maximum,f32 start,f32 end){
 if(value<=minimum)return start;
 if(value>=maximum)return end;
 return start+(end-start)*(value-minimum)/(maximum-minimum);
}
}

void JAUAudience3DSetting::init(){
 mFarDistance=7000.0f;
 mNearDistance=800.0f;
 mMinVolume=0.005f;
 mBehindDolbyZ=8000.0f;
 mFrontDolbyZ=-4000.0f;
 mMinFxMix=0.0f;
 mMaxFxMix=0.1f;
 mMaxDolby=1.0f;
 mCenterDolby=0.8f;
 mMinDolby=0.0f;
 mPanAmplitude=0.5f;
 mPriorityDownRange=255;
 mSonicSpeed=3000.0f;
 mPitchDeltaRatioRange=1.5f;
}
