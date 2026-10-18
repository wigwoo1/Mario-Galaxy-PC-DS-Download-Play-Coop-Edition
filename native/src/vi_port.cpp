#include <Native/VideoBinding.hpp>
namespace {
Native::VideoInterface* active=nullptr;
Native::VideoInterface& video(){if(!active)throw std::logic_error("No active native VI binding");return *active;}
}
namespace Native {
VideoBinding::VideoBinding(VideoInterface& state):previous(active){if(active)throw std::logic_error("Nested native VI binding");active=&state;}
VideoBinding::~VideoBinding(){active=previous;}
}
extern "C" {
void VIInit(){(void)video();}
void VIConfigure(const GXRenderModeObj* mode){if(!mode)throw std::invalid_argument("Null VI mode");video().configure(*mode);}
void VIFlush(){video().flush();}
void VISetBlack(BOOL black){if(black!=0&&black!=1)throw std::invalid_argument("Invalid VI black flag");video().setBlack(black!=0);}
// RF trap filtering is a Wii analog-output feature and has no desktop-display equivalent.
void VISetTrapFilter(VIBool filter){if(filter!=0&&filter!=1)throw std::invalid_argument("Invalid VI trap-filter flag");}
void VISetNextFrameBuffer(void* buffer){video().setNextFramebuffer(buffer);}
void* VIGetNextFrameBuffer(){return video().nextFramebuffer();}
void* VIGetCurrentFrameBuffer(){return video().current().framebuffer;}
u32 VIGetRetraceCount(){return video().retraceCount();}
BOOL VIEnableDimming(BOOL enable){return video().setDimmingEnabled(enable);}
u32 VIGetDimmingCount(){return video().dimmingCount();}
VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback cb){return video().setPre(cb);}
VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback cb){return video().setPost(cb);}
void VIWaitForRetrace(){
 if(!video().waitForRetrace(std::chrono::milliseconds(2000)))
  throw std::runtime_error("Native VI retrace scheduler did not advance");
}
u32 VIGetScanMode(){auto state=video().current();if(!state.configured)throw std::logic_error("VI mode not yet latched");return state.mode.viTVmode&3;}
}
extern "C" void nativeNotifyVideoActivity(){if(active)active->notifyActivity();}
