#pragma once
#include <JSystem/JAudio2/JAUSoundAnimator.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

// The original interface declares a pure virtual destructor without a body.
// Native BAS storage implements that interface and needs its base destructor.
inline JAUSoundAnimationControl::~JAUSoundAnimationControl() {}

namespace Native {
class BasAnimation final : public JAUSoundAnimationControl {
    JAUSoundAnimation animation_{};
    std::vector<JAUSoundAnimationSound> sounds_;
public:
    BasAnimation(const void* source,std::size_t size) {
        if(!source||size<8)throw std::invalid_argument("Missing or truncated BAS header");
        const auto* bytes=static_cast<const std::uint8_t*>(source);
        auto word=[&](std::size_t at){return std::uint32_t(bytes[at])<<24|std::uint32_t(bytes[at+1])<<16|std::uint32_t(bytes[at+2])<<8|bytes[at+3];};
        auto scalar=[&](std::size_t at){const auto bits=word(at);float value;std::memcpy(&value,&bits,4);if(!std::isfinite(value))throw std::invalid_argument("Non-finite BAS sound parameter");return value;};
        const auto count=std::uint16_t(unsigned(bytes[0])<<8|bytes[1]);
        if(word(4)!=0)throw std::invalid_argument("Serialized BAS control pointer is not supported");
        if(count>(size-8)/32)throw std::out_of_range("Truncated BAS sound records");
        sounds_.resize(count);
        for(std::size_t i=0;i<count;++i) {
            const auto at=8+i*32;auto& sound=sounds_[i];
            sound.mSoundID=JAISoundID(word(at));
            sound.mNoteOnTime=scalar(at+4);sound.mNoteOffTime=scalar(at+8);sound.mBasePitch=scalar(at+12);
            sound.mFlags=word(at+16);sound.mBaseVolume=bytes[at+20];sound.mPitchDelta=static_cast<s8>(bytes[at+21]);
            sound.mPlayTime=bytes[at+22];sound.mBasePan=bytes[at+23];sound.mVolumeDelta=static_cast<s8>(bytes[at+24]);
            sound.mRepeatInterval=bytes[at+25];sound._1A=static_cast<s8>(bytes[at+26]);sound._1C=word(at+28);
            if(sound.playsAtIntervals()&&!sound.mRepeatInterval)throw std::invalid_argument("BAS repeat interval is zero");
        }
        animation_.mNumSounds=count;
        // The Wii image has a 32-bit pointer slot and cannot be overlaid on the
        // host class. Its existing control interface provides a real array.
        animation_.mControl=this;
    }
    BasAnimation(const BasAnimation&)=delete;
    BasAnimation& operator=(const BasAnimation&)=delete;
    JAUSoundAnimation* animation(){return &animation_;}
    JAUSoundAnimationSound* getSound(const JAUSoundAnimation*,int index) override {
        if(index<0||std::size_t(index)>=sounds_.size())throw std::out_of_range("BAS sound index");
        return &sounds_[index];
    }
    u16 getNumSounds(const JAUSoundAnimation*) override{return static_cast<u16>(sounds_.size());}
};
}
