#include <Game/Speaker/SpkSound.hpp>
#include <cmath>
#include <Game/Speaker/SpkMixingBuffer.hpp>
#include <cassert>
#include <cstdio>
// Fixture-only storage: original allocation and speaker-device update are not invoked.
SpkMixingBuffer::SpkMixingBuffer(JKRHeap*){}
int main(){
 bool kill=true;SpkSoundVolume volume;
 assert(volume.calc(kill)==1&&!kill);volume.setRelease(4);
 const float release[]={.5625f,.25f,.0625f,0};
 for(int i=0;i<4;++i){assert(volume.calc(kill)==release[i]);assert(kill==(i==3));}
 assert(volume.calc(kill)==0&&!kill);
 volume=SpkSoundVolume();volume.setRelease(0);assert(volume.calc(kill)==0&&kill);
 volume=SpkSoundVolume();volume.mFadeOutRate=.5f;
 assert(volume.calc(kill)==.5f&&!kill);assert(volume.calc(kill)==0&&kill);
 volume=SpkSoundVolume();volume.mFadeVolume=.2f;volume.mFadeTarget=.5f;volume.mFadeRate=.2f;
 assert(std::fabs(volume.calc(kill)-.4f)<.00001f&&!kill);
 assert(volume.calc(kill)==.5f&&volume.mFadeRate==0&&!kill);
 volume.mFadeTarget=.1f;volume.mFadeRate=-.3f;volume.calc(kill);
 assert(volume.calc(kill)==.1f&&volume.mFadeRate==0&&!kill);
 volume=SpkSoundVolume();volume.mBaseVolume=.5f;volume.mFadeVolume=.5f;
 assert(volume.calc(kill)==.25f&&!kill);
 std::puts("PASS original speaker volume: squared release/completion pulse/immediate stop/fadeout/signed target clamps/base gain");
 SpkMixingBuffer mixer(nullptr);s16 storage[4][40]{};
 for(int c=0;c<4;++c)mixer.mSamples[c]=storage[c];
 s16 wave[]={1000,-1000,32767,-32768};
 mixer.mix(1,wave,4,.5f,3);
 assert(storage[1][3]==500&&storage[1][4]==-500&&storage[1][5]==16383&&storage[1][6]==-16384);
 assert(storage[1][2]==0&&storage[1][7]==0&&storage[0][3]==0);
 mixer.mix(1,wave,4,1.0f,3);
 assert(storage[1][3]==1500&&storage[1][4]==-1500&&storage[1][5]==32767&&storage[1][6]==-32768);
 s16 odd[]={3,-3};mixer.mix(2,odd,2,.5f,38);
 assert(storage[2][38]==1&&storage[2][39]==-1&&storage[1][38]==0);
 mixer.mix(2,wave,0,1,0);assert(storage[2][0]==0);
 mixer.mix(3,wave,4,0,0);assert(storage[3][0]==0&&storage[3][3]==0);
 assert(mixer.getSamples(1)==storage[1]);
 std::puts("PASS original speaker PCM: scaled signed/additive/saturation/channel isolation/offset/truncation/zero volume");
}

