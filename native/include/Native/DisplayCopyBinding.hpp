#pragma once
#include <Native/GxCommandObservation.hpp>
#include <cstddef>
#include <cstdint>
namespace Native {
class GxCommandState;class GpuResources;
const GxCommandState* currentDisplayCopyCommandState();
class DisplayCopyBinding {
public:
 DisplayCopyBinding(GxCommandState&,GpuResources&);
 ~DisplayCopyBinding();
 DisplayCopyBinding(const DisplayCopyBinding&)=delete;
 DisplayCopyBinding& operator=(const DisplayCopyBinding&)=delete;
 GxCommandState& commandState()const{return *state;}
 GpuResources& resourceStore()const{return *resources;}
 void observe(const std::uint8_t*,std::size_t,bool);
private:
 GxCommandState* state;
 GpuResources* resources;
 DisplayCopyBinding* previousBinding;
 GxCommandObserver previousObserver;
 const void* observedList=nullptr;
 const std::uint8_t* observedStart=nullptr;
 std::size_t observedBytes=0;
 friend void observeDisplayCopyCommands(const std::uint8_t*,std::size_t,bool);
};
}
