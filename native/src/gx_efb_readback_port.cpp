#include <Native/EfbReadback.hpp>
#include <revolution/gx/GXCpu2Efb.h>
#include <cstdint>
#include <stdexcept>

namespace {
void* activeContext=nullptr;
Native::EfbPixelReader activeReader=nullptr;
unsigned alphaReadMode=GX_READ_FF;
}

namespace Native {
EfbReadbackBinding::EfbReadbackBinding(void* context,EfbPixelReader reader)
 :previousContext(activeContext),previousReader(activeReader){
 if(!context||!reader)throw std::invalid_argument("Invalid EFB readback binding");
 activeContext=context;activeReader=reader;
}
EfbReadbackBinding::~EfbReadbackBinding(){activeContext=previousContext;activeReader=previousReader;}
std::uint32_t peekEfbArgb(std::uint16_t x,std::uint16_t y){
 if(!activeContext||!activeReader)throw std::logic_error("GX EFB readback is not bound to the active host renderer");
 std::uint32_t color=activeReader(activeContext,x,y);
 if(alphaReadMode==GX_READ_FF)color|=0xff000000u;
 else if(alphaReadMode==GX_READ_00)color&=0x00ffffffu;
 return color;
}
void setEfbAlphaReadMode(unsigned mode){if(mode>GX_READ_NONE)throw std::invalid_argument("GX alpha read mode");alphaReadMode=mode;}
unsigned efbAlphaReadMode(){return alphaReadMode;}
}

extern "C" void GXPokeAlphaRead(GXAlphaReadMode mode){Native::setEfbAlphaReadMode(unsigned(mode));}
extern "C" void GXPeekARGB(u16 x,u16 y,u32* color){
 if(!color)throw std::invalid_argument("Null GX pixel-read destination");
 *color=Native::peekEfbArgb(x,y);
}
