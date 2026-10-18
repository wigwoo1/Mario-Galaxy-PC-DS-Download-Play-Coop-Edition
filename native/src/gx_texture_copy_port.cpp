#include <Native/TextureCopyBinding.hpp>
#include <Native/DisplayCopyBinding.hpp>
#include <Native/GxCommandState.hpp>
#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <stdexcept>
void emitNativeTextureCopy(void*,GXBool,const Native::GxCommandState&,Native::GpuResources&);
namespace {const Native::GxCommandState* state=nullptr;Native::GpuResources* resources=nullptr;}
namespace Native {
TextureCopyBinding::TextureCopyBinding(const GxCommandState& current,GpuResources& storage):previousState(state),previousResources(resources){state=&current;resources=&storage;}
TextureCopyBinding::~TextureCopyBinding(){state=previousState;resources=previousResources;}
}
extern "C" void GXCopyTex(void* destination,GXBool clear){
 if(!state||!resources)throw std::logic_error("GXCopyTex requires active native render state");
 if(__GDCurrentDL)Native::notifyGxCommandObserver(__GDCurrentDL->start,GDGetCurrOffset());
 emitNativeTextureCopy(destination,clear,*state,*resources);
 if(__GDCurrentDL)Native::notifyGxCommandObserver(__GDCurrentDL->start,GDGetCurrOffset());
}
extern "C" void GXPixModeSync(){
 const auto* syncState=Native::currentDisplayCopyCommandState();
 if(!syncState)syncState=state;
 if(!syncState)throw std::logic_error("GXPixModeSync requires active native render state");
 GDOverflowCheck(5);GDWriteBPCmd(0x43000000|syncState->bp(0x43));
}
