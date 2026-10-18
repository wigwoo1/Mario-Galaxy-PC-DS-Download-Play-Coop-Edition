#include <Native/KpadInput.hpp>
#include <array>
#include <mutex>
#include <cmath>
namespace {
struct Channel {std::array<KPADStatus,120> samples{};size_t count{};u32 held{};double lastTime{-1},nextRepeat{};float delay{0.5f},pulse{1.0f/6.0f};NativeKpadCalibration calibration{};};
std::array<Channel,4> channels;
std::mutex inputMutex;
bool valid(s32 channel){return channel>=0&&channel<4;}
}
bool nativeSubmitKpadSample(s32 channel,const KPADStatus& sample){
 if(!valid(channel))return false;
 std::lock_guard<std::mutex> lock(inputMutex);auto& queue=channels[channel];
 if(queue.count==queue.samples.size())return false;
 queue.samples[queue.count++]=sample;return true;
}
extern "C" void KPADReset(){std::lock_guard<std::mutex> lock(inputMutex);for(auto& c:channels){c.count=0;c.held=0;c.lastTime=-1;c.nextRepeat=0;}}
extern "C" void KPADInit(){KPADReset();}
extern "C" s32 KPADRead(s32 channel,KPADStatus* output,u32 capacity){
 if(!valid(channel)||!output||!capacity)return 0;
 std::lock_guard<std::mutex> lock(inputMutex);auto& queue=channels[channel];
 // The game treats sample0 as the current state; return newest first.
 const size_t count=capacity<queue.count?capacity:queue.count;
 for(size_t i=0;i<count;++i)output[i]=queue.samples[queue.count-1-i];
 queue.count-=count;return static_cast<s32>(count);
}
extern "C" void KPADSetBtnRepeat(s32 channel,f32 delay,f32 pulse){
 if(!valid(channel)||!std::isfinite(delay)||!std::isfinite(pulse)||delay<0||pulse<=0)return;
 std::lock_guard<std::mutex> lock(inputMutex);auto& c=channels[channel];c.delay=delay;c.pulse=pulse;
 if(c.lastTime>=0)c.nextRepeat=c.lastTime+delay;
}
bool nativeSubmitKpadState(s32 channel,const KPADStatus& state,double seconds){
 if(!valid(channel)||!std::isfinite(seconds)||seconds<0)return false;
 std::lock_guard<std::mutex> lock(inputMutex);auto& c=channels[channel];
 if(c.count==c.samples.size()||seconds<c.lastTime)return false;
 KPADStatus sample=state;
 const u32 held=state.wpad_err==WPAD_ERR_NONE||state.wpad_err==WPAD_ERR_BUSY ? state.hold&KPAD_BUTTON_MASK : 0;
 sample.hold=held;sample.trig=held&~c.held;sample.release=c.held&~held;
 if(held!=c.held)c.nextRepeat=seconds+c.delay;
 else if(held&&seconds>=c.nextRepeat){
  sample.hold|=KPAD_BUTTON_RPT;
  c.nextRepeat+= (std::floor((seconds-c.nextRepeat)/c.pulse)+1)*c.pulse;
 }
 c.held=held;c.lastTime=seconds;c.samples[c.count++]=sample;return true;
}
bool nativeReadKpadCalibration(s32 channel,NativeKpadCalibration* output){
 if(!valid(channel)||!output)return false;
 std::lock_guard<std::mutex> lock(inputMutex);*output=channels[channel].calibration;return true;
}
extern "C" void KPADSetAccParam(s32 channel,f32 radius,f32 sensitivity){
 if(!valid(channel))return;std::lock_guard<std::mutex> lock(inputMutex);auto& c=channels[channel].calibration;c.accelerationRadius=radius;c.accelerationSensitivity=sensitivity;
}
extern "C" void KPADSetPosParam(s32 channel,f32 radius,f32 sensitivity){
 if(!valid(channel))return;std::lock_guard<std::mutex> lock(inputMutex);auto& c=channels[channel].calibration;c.posRadius=radius;c.posSensitivity=sensitivity;
}
extern "C" void KPADSetHoriParam(s32 channel,f32 radius,f32 sensitivity){
 if(!valid(channel))return;std::lock_guard<std::mutex> lock(inputMutex);auto& c=channels[channel].calibration;c.horizonRadius=radius;c.horizonSensitivity=sensitivity;
}
extern "C" void KPADSetDistParam(s32 channel,f32 radius,f32 sensitivity){
 if(!valid(channel))return;std::lock_guard<std::mutex> lock(inputMutex);auto& c=channels[channel].calibration;c.distanceRadius=radius;c.distanceSensitivity=sensitivity;
}
extern "C" void KPADSetSensorHeight(s32 channel,f32 height){
 if(!valid(channel))return;std::lock_guard<std::mutex> lock(inputMutex);channels[channel].calibration.sensorHeight=height;
}
