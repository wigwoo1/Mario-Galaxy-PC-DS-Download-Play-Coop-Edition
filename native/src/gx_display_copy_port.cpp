#include <Native/DisplayCopyBinding.hpp>
#include <Native/GxCommandState.hpp>
#include <Native/GpuResources.hpp>
#include <Native/GxCommandObservation.hpp>
#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <stdexcept>
void emitNativeDisplayCopy(void*,GXBool,const Native::GxCommandState&,Native::GpuResources&);
namespace {Native::DisplayCopyBinding* activeBinding=nullptr;}
namespace Native {
const GxCommandState* currentDisplayCopyCommandState(){return activeBinding?&activeBinding->commandState():nullptr;}
void observeDisplayCopyCommands(const std::uint8_t* start,std::size_t size,bool reset){if(activeBinding)activeBinding->observe(start,size,reset);}
DisplayCopyBinding::DisplayCopyBinding(GxCommandState& current,GpuResources& storage):state(&current),resources(&storage),previousBinding(activeBinding),previousObserver(exchangeGxCommandObserver(&observeDisplayCopyCommands)){activeBinding=this;}
DisplayCopyBinding::~DisplayCopyBinding(){activeBinding=previousBinding;exchangeGxCommandObserver(previousObserver);}
void DisplayCopyBinding::observe(const std::uint8_t* start,std::size_t size,bool reset){
 if(reset){observedList=nullptr;observedStart=nullptr;observedBytes=0;return;}
 if(!__GDCurrentDL||!start||start!=__GDCurrentDL->start)return;
 if(observedList!=__GDCurrentDL||observedStart!=start||size<observedBytes){observedList=__GDCurrentDL;observedStart=start;observedBytes=0;}
 if(size>observedBytes){state->consume(start+observedBytes,size-observedBytes,resources);observedBytes=size;}
 state->clearPendingCopies();state->clearPendingSyncEvents();
}
}
extern "C" void GXCopyDisp(void* destination,GXBool clear){
 if(!activeBinding)throw std::logic_error("GXCopyDisp requires active native render state");
 if(__GDCurrentDL)Native::notifyGxCommandObserver(__GDCurrentDL->start,GDGetCurrOffset());
 emitNativeDisplayCopy(destination,clear,activeBinding->commandState(),activeBinding->resourceStore());
 if(__GDCurrentDL)Native::notifyGxCommandObserver(__GDCurrentDL->start,GDGetCurrOffset());
}
