#include <JSystem/JAudio2/JASDSPInterface.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <cassert>
#include <cstdio>
extern "C" void DCFlushRange(void*,u32){}
int main(){
 JASWaveInfo wave{};assert(wave.mBaseKey==60&&wave._24==&JASWaveInfo::one&&*wave._24==1);
 wave.mSampleCount=1000;wave.mSampleLoopStart=128;wave.mSampleLoopEnd=900;wave.mpLastSample=-123;wave.mpPenultSample=456;
 const unsigned blockSamples[8]={16,16,1,1,1,16,16,1};const unsigned blockBytes[8]={9,5,8,16,1,1,1,1};
 for(unsigned fmt=0;fmt<8;++fmt){wave.mFormat=fmt;wave.mLoopFlags=1;JASDsp::TChannel c{};c.setWaveInfo(wave,0x100000,1);assert(c._64==blockSamples[fmt]&&c._100==blockBytes[fmt]);if(fmt<4){assert(c._102==1&&c._104==-123&&c._106==456&&c._11C==1000);assert(c._68==128);if(fmt<2){assert(c._118==0x100000+128*blockBytes[fmt]/16&&c._110==0&&c._114==772);}else{assert(c._118==0x100000&&c._110==128&&c._114==900);}}else{assert(c._118==0x100000&&c._68==0);}}
 wave.mFormat=0;wave.mLoopFlags=0;JASDsp::TChannel c{};c.setWaveInfo(wave,0x200000,0);assert(c._114==1000&&c._102==0&&c._68==0);wave.mLoopFlags=1;c.setWaveInfo(wave,0x200000,900);assert(c._68==0&&c._118==0x200000);c.setOscInfo(7);assert(c._118==0&&c._64==16&&c._100==7);JASWaveInfo copy{};copy.copy(wave);assert(copy._24==wave._24&&copy.mFormat==wave.mFormat&&copy.mSampleLoopEnd==900&&copy.mpLastSample==-123);puts("PASS original wave formats, ADPCM offsets, loop metadata, nonloop bounds, oscillator setup and shared metadata identity.");
}
