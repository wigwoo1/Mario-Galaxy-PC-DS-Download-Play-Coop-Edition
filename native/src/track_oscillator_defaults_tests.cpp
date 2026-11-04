#include <Native/HeapPlacement.hpp>
#include <JSystem/JAudio2/JASTrack.hpp>
#include <JSystem/JAudio2/JASSeqParser.hpp>
#include <JSystem/JAudio2/JASDriverIF.hpp>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <new>

// Link-only guards: the real track constructor/initializer are exercised, but
// this focused fixture never executes sequence bytecode or schedules notes.
s32 JASSeqParser::parse(JASTrack*) {assert(false&&"Unexpected sequence execution");return -1;}
bool JASSeqParser::execNoteOnMidi(JASTrack*,u32,u32,u32) {assert(false);return false;}
void JASSeqParser::execNoteOnGate(JASTrack*,u32,u32,u32,u32) {assert(false);}
void JASSeqParser::execNoteOff(JASTrack*,u32) {assert(false);}
s32 JASSeqParser::execCommand(JASTrack*,CommandFunc,u32,u32*) {assert(false);return -1;}

void unity(JASTrack& track) {
    for(unsigned i=0;i<2;++i) {
        const auto& data=track.mOscParam[i];JASOscillator oscillator;oscillator.initStart(&data);
        std::printf("oscillator%u recordBytes=%zu target=%u scale=%.9g bias=%.9g value=%.9g\n",i,sizeof(data),data.mTarget,data.mScale,data._14,oscillator.getValue());
        assert(data.mTarget==i&&!data.mTable&&!data.rel_table);
        assert(oscillator.getValue()==1.0f&&"Native track defaults leave the oscillator record tail uninitialized");
    }
}
int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    static_assert(sizeof(JASOscillator::Data)==32,"This fixture targets the current x64 oscillator ABI");
    static_assert(sizeof(JASTrack)==808&&sizeof(JASTrack::TList)==32,"Existing native track/list ABI changed");
    for(unsigned char poison:{0xa5,0xcd,0xff,0x7f}) {
        alignas(JASTrack) std::array<unsigned char,sizeof(JASTrack)> storage;
        std::memset(storage.data(),poison,storage.size());
        auto* track=::new(static_cast<void*>(storage.data())) JASTrack;
        unity(*track);
        {
            auto* mapped=JASTrack::TList::TrackList::Element_toNode(track);
            const auto offset=reinterpret_cast<unsigned char*>(&track->mNode)-reinterpret_cast<unsigned char*>(track);
            std::printf("trackBytes=%zu listBytes=%zu actualNodeOffset=%zu mappedNodeOffset=%zu\n",sizeof(JASTrack),sizeof(JASTrack::TList),size_t(offset),size_t(reinterpret_cast<unsigned char*>(mapped)-reinterpret_cast<unsigned char*>(track)));
            assert(mapped==&track->mNode&&"Track list still converts nodes using the Wii object offset");
            assert(JASTrack::TList::TrackList::Element_toValue(&track->mNode)==track);
            JASTrack second;JASTrack::TList list;
            list.append(track);list.append(&second);
            std::printf("list length=%u registered=%d end=%p next=%p firstNode=%p secondNode=%p beginValue=%p firstValue=%p\n",list.mTracks.size(),list.mCallbackRegistered,(void*)&list.mTracks.mEnd,(void*)list.mTracks.mEnd.mNext,(void*)&track->mNode,(void*)&second.mNode,(void*)*list.mTracks.begin(),(void*)track);
            auto it=list.mTracks.begin();assert(it!=list.mTracks.end()&&*it==track);
            ++it;assert(it!=list.mTracks.end()&&*it==&second);++it;assert(it==list.mTracks.end());
            list.mTracks.Remove(track);assert(*list.mTracks.begin()==&second);
            list.mTracks.Remove(&second);assert(list.mTracks.begin()==list.mTracks.end());
            assert(JASDriver::rejectCallback(JASTrack::TList::cbSeqMain,&list)==1);
        }
        track->mOscParam[0].mScale=-3;track->mOscParam[0]._14=.75f;
        track->mOscParam[1].mScale=8;track->mOscParam[1]._14=-2;
        track->init();unity(*track);
        const auto pitch=track->mOscParam[1];
        track->mOscParam[0].mScale=-9;track->mOscParam[0]._14=.5f;
        track->setOscAdsr(10,20,30,16000,0);
        assert(track->mOscParam[0].mScale==1&&track->mOscParam[0]._14==0);
        assert(track->mOscParam[0].mTable==track->mOscPoint&&!track->mOscParam[0].rel_table);
        assert(track->mOscParam[1].mTarget==pitch.mTarget&&track->mOscParam[1].mScale==pitch.mScale&&track->mOscParam[1]._14==pitch._14);
        JASOscillator envelope;envelope.initStart(&track->mOscParam[0]);
        for(unsigned i=1;i<=64;++i) {
            envelope.incCounter(1);const float value=envelope.getValue();assert(std::isfinite(value)&&value>=0&&value<=1);
            if(i<10)assert(std::fabs(value-float(i)*(32767.0f/32768.0f)/10.0f)<.00001f);
        }
        assert(std::fabs(envelope.getValue()-16000.0f/32768.0f)<.00001f);
        envelope.release();assert(envelope.isStop());track->~JASTrack();
    }
    std::puts("PASS actual track construction/reinit/ADSR defaults on four poisoned object images; unity pitch/volume, original64-tick envelope and native node roundtrip/two-track append/iteration/removal; no sequence execution");
}
