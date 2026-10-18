#include <Native/DisplayConfig.hpp>
#include <revolution/sc.h>
#include <revolution/vi.h>
#include <mutex>
#include <stdexcept>
namespace {
std::mutex configMutex;
Native::DisplayConfig config;
}
namespace Native {
DisplayConfig displayConfig(){std::lock_guard<std::mutex> lock(configMutex);return config;}
void setDisplayConfig(const DisplayConfig& value){
    if(value.televisionFormat!=VI_NTSC && value.televisionFormat!=VI_PAL &&
       value.televisionFormat!=VI_MPAL && value.televisionFormat!=VI_EURGB60)
        throw std::invalid_argument("Unsupported original television format");
    std::lock_guard<std::mutex> lock(configMutex);config=value;
}
}
extern "C" {
u8 SCGetAspectRatio(){return Native::displayConfig().widescreen?SC_ASPECT_RATIO_16x9:SC_ASPECT_RATIO_4x3;}
u8 SCGetProgressiveMode(){return Native::displayConfig().progressive?SC_PROGRESSIVE_MODE_ON:SC_PROGRESSIVE_MODE_OFF;}
u8 SCGetEuRgb60Mode(){return Native::displayConfig().euRgb60?SC_EURGB60_MODE_ON:SC_EURGB60_MODE_OFF;}
u32 VIGetTvFormat(){return Native::displayConfig().televisionFormat;}
u32 VIGetDTVStatus(){return Native::displayConfig().digitalOutput?1:0;}
}
