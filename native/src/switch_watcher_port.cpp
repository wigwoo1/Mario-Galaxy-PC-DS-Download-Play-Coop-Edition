#include "Game/Map/StageSwitch.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <array>
#include <functional>
#include <map>
#include <stdexcept>
namespace {std::map<const StageSwitchCtrl*,std::function<bool(int,bool)>> ranges;std::map<const StageSwitchCtrl*,std::array<bool,4>> validity;std::map<const StageSwitchCtrl*,std::array<std::function<void(bool)>,3>> writers;std::map<const StageSwitchCtrl*,std::array<std::function<bool()>,3>> bindings;bool read(const StageSwitchCtrl* control,size_t channel){auto found=bindings.find(control);if(found==bindings.end())throw std::logic_error("Unbound native StageSwitchCtrl");return found->second[channel]();}}
StageSwitchCtrl::StageSwitchCtrl(const JMapInfoIter& iter):mSW_A(nullptr),mSW_B(nullptr),mSW_Appear(nullptr),mSW_Dead(nullptr){if(iter.isValid())throw std::logic_error("Original placement StageSwitchCtrl construction unavailable");}
void bindNativeStageSwitchCtrl(const StageSwitchCtrl* control,std::array<std::function<bool()>,3> readers,std::array<bool,4> valid){if(!bindings.emplace(control,std::move(readers)).second)throw std::logic_error("Duplicate native stage switch binding");try{validity.emplace(control,valid);}catch(...){bindings.erase(control);throw;}}
void bindNativeStageSwitchWrites(const StageSwitchCtrl* control,std::array<std::function<void(bool)>,3> callbacks){if(!writers.emplace(control,std::move(callbacks)).second)throw std::logic_error("Duplicate native switch write binding");}
void unbindNativeStageSwitchCtrl(const StageSwitchCtrl* control){bindings.erase(control);writers.erase(control);validity.erase(control);ranges.erase(control);}
namespace {void write(const StageSwitchCtrl* control,size_t channel,bool enabled){auto found=writers.find(control);if(found==writers.end())throw std::logic_error("Unbound native switch write");found->second[channel](enabled);}}
void StageSwitchCtrl::onSwitchA(){write(this,0,true);}void StageSwitchCtrl::offSwitchA(){write(this,0,false);}void StageSwitchCtrl::onSwitchB(){write(this,1,true);}void StageSwitchCtrl::offSwitchB(){write(this,1,false);}
bool StageSwitchCtrl::isOnSwitchA()const{return read(this,0);}
bool StageSwitchCtrl::isOnSwitchB()const{return read(this,1);}
bool StageSwitchCtrl::isOnSwitchAppear()const{return read(this,2);}


namespace {bool valid(const StageSwitchCtrl* control,size_t channel){auto found=validity.find(control);return found!=validity.end()&&found->second[channel];}}
bool StageSwitchCtrl::isValidSwitchA()const{return valid(this,0);}bool StageSwitchCtrl::isValidSwitchB()const{return valid(this,1);}bool StageSwitchCtrl::isValidSwitchAppear()const{return valid(this,2);}

void StageSwitchCtrl::onSwitchDead(){write(this,2,true);}void StageSwitchCtrl::offSwitchDead(){write(this,2,false);}bool StageSwitchCtrl::isValidSwitchDead()const{return valid(this,3);}

// Original BitFlag128 method subset from pinned StageSwitch.cpp.
bool BitFlag128::get(int idx) const {
    u32 flags = mFlags[idx / 32];
    u32 mask = (1U << (idx % 32));
    return mask == (flags & static_cast<u32>(mask));
}

void BitFlag128::set(int bitIndex, bool setTrue) {
    u32 valIdx = bitIndex / 32;
    u32& flag = mFlags[valIdx];

    if (setTrue) {
        flag |= (1U << (bitIndex % 32));
    } else {
        flag &= ~(1U << (bitIndex % 32));
    }
}


void bindNativeStageSwitchRange(const StageSwitchCtrl* control,std::function<bool(int,bool)> callback){if(!ranges.emplace(control,std::move(callback)).second)throw std::logic_error("Duplicate native switch range binding");}
namespace {bool rangeQuery(const StageSwitchCtrl* control,int count,bool all){auto found=ranges.find(control);if(found==ranges.end())throw std::logic_error("Unbound native switch range");return found->second(count,all);}}
bool StageSwitchCtrl::isOnAllSwitchAfterB(int count)const{return rangeQuery(this,count,true);}
bool StageSwitchCtrl::isOnAnyOneSwitchAfterB(int count)const{return rangeQuery(this,count,false);}
