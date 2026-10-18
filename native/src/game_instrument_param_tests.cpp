#include <JSystem/JAudio2/JASBasicInst.hpp>
#include <cassert>
#include <cstdio>
int main(){JASInstParam p;std::fprintf(stderr,"defaults %g %g %g %g %g type %u wave %u osc %p count %u flag %u release %u\n",p.mVolume,p.mPitch,p.mPan,p.mFxMix,p.mDolby,p.mChannelType,p._18,p.mOscillatorData,p.mNumOscillators,p._24,p.mDirectRelease);if(!(p.mVolume==1&&p.mPitch==1&&p.mPan==.5f&&p.mFxMix==0&&p.mDolby==0))return 1;if(!(p.mChannelType==JASChannel::CH_WAVE&&p._18==0&&p.mOscillatorData==nullptr&&p.mNumOscillators==0&&!p._24&&p.mDirectRelease==0))return 2;puts("PASS original game instrument defaults and inherited sound parameters.");}

