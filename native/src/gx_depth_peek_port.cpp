#include <Native/DepthPeek.hpp>
#include <revolution/gx/GXCpu2Efb.h>
#include <stdexcept>
namespace {thread_local Native::DepthPeekSource* source=nullptr;}
Native::DepthPeekBinding::DepthPeekBinding(DepthPeekSource& value):previous(source){source=&value;}
Native::DepthPeekBinding::~DepthPeekBinding(){source=previous;}
extern "C" void GXPeekZ(u16 x,u16 y,u32* value){
 if(!value)throw std::invalid_argument("Null GX depth output");
 if(!source)throw std::logic_error("No native depth source on calling render thread");
 auto result=source->peek(x,y);*value=result;
}

