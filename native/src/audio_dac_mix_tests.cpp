#include <JSystem/JAudio2/JASAiCtrl.hpp>
#include <JSystem/JAudio2/JASProbe.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
extern "C" void DCZeroRange(void* p,u32 n){std::memset(p,0,n);} extern "C" void DCInvalidateRange(void*,u32){}
void JASProbe::start(s32,const char*){} void JASProbe::stop(s32){}
s16 samples[1600];s16* supply(s32){return samples;}s16* empty(s32){return nullptr;}
int main(){JASDriver::registerMixCallback(supply,MIX_MODE_INTERLEAVE);assert(JASDriver::extMixCallback==supply&&JASDriver::sMixMode==MIX_MODE_INTERLEAVE);JASDriver::registerMixCallback(nullptr,MIX_MODE_EXTRA);assert(!JASDriver::extMixCallback&&JASDriver::sMixMode==MIX_MODE_EXTRA);JASDriver::registDSPBufCallback(nullptr);assert(!JASDriver::sDspDacCallback);s16 ring0[]={1,2,3,4,5,6,7,8},ring1[]={11,12,13,14,21,22,23,24},ring2[8]{};s16* ring[]={ring0,ring1,ring2};JASDriver::sDspDacBuffer=ring;JASDriver::sDspDacReadBuffer=0;JASDriver::sDspDacWriteBuffer=2;s16 output[9]{};output[8]=1234;JASDriver::readDspBuffer(output,4);assert(JASDriver::sDspDacReadBuffer==1);for(int i=0;i<4;++i){assert(output[2*i]==ring1[4+i]&&output[2*i+1]==ring1[i]);}assert(output[8]==1234);JASDriver::sDspDacReadBuffer=0;JASDriver::sDspDacWriteBuffer=1;JASDriver::readDspBuffer(output,4);assert(JASDriver::sDspDacReadBuffer==0);for(int i=0;i<4;++i){assert(output[2*i]==4&&output[2*i+1]==2);}assert(output[8]==1234);JASDriver::setOutputRate(static_cast<JASOutputRate>(0));assert(JASDriver::getSubFrames()==7&&JASDriver::getFrameSamples()==560&&JASDriver::getDacSize()==1120);assert(std::fabs(JASDriver::getDacRate()-32028.4704f)<.01f);JASDriver::setOutputRate(static_cast<JASOutputRate>(1));assert(JASDriver::getSubFrames()==10&&JASDriver::getFrameSamples()==800);s16 b[]={32000,-32000,123,456};samples[0]=2000;samples[1]=-2000;JASDriver::mixMonoTrack(b,2,supply);assert(b[0]==32767&&b[1]==-30000&&b[2]==-1877&&b[3]==-1544);s16 w[]={32000,-32000};JASDriver::mixMonoTrackWide(w,1,supply);assert(w[0]==32767&&w[1]==-32768);s16 inter[]={100,-100};samples[0]=10;samples[1]=20;JASDriver::mixInterleaveTrack(inter,1,supply);assert(inter[0]==110&&inter[1]==-80);JASDriver::mixInterleaveTrack(inter,1,empty);assert(inter[0]==110&&inter[1]==-80);samples[0]=30;samples[800]=40;s16 planar[]={1,2};JASDriver::mixExtraTrack(planar,1,supply);assert(planar[0]==41&&planar[1]==32);puts("PASS original DAC rates/sizes and mono/wide/interleaved/planar saturating mixing (profiling fixture).");}



