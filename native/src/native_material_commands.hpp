#pragma once
#include <JSystem/J3DGraphBase/J3DMaterial.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
#include <Native/GxCommandState.hpp>
#include <vector>
#include <stdexcept>
#include <revolution/os/OSInterrupt.h>

// Retains original emitted material commands after the factory and its decoded
// input tables go away. GPU resource registrations must remain live while the
// commands are used. Geometry submission remains a separate renderer step.
class NativeMaterialCommands {
 std::vector<uint8_t> bytes_;
 Native::GxCommandState state_;
public:
 NativeMaterialCommands(J3DMaterial& material,JKRHeap& heap){
  struct Guard {
   BOOL prior=OSDisableInterrupts();GDLObj* previous=__GDCurrentDL;
   ~Guard(){GDSetCurrent(previous);OSRestoreInterrupts(prior);}
  } guard;
  auto size=material.countDLSize();
  if(!size || size>1024*1024)throw std::runtime_error("Invalid material command capacity");
  auto* storage=heap.alloc(size,32);
  if(!storage)throw std::bad_alloc();
  try{
   J3DDisplayListObj list;list.setSingleDisplayList(storage,size);
   material.makeDisplayList_private(&list);
   if(!list.mSize || list.mSize>size)throw std::runtime_error("Invalid emitted material extent");
   auto* first=static_cast<uint8_t*>(storage);bytes_.assign(first,first+list.mSize);
   state_.consume(bytes_.data(),bytes_.size(),&Native::gpuResources());
  }catch(...){JKRHeap::free(storage,&heap);throw;}
  JKRHeap::free(storage,&heap);
 }
 const std::vector<uint8_t>& bytes()const{return bytes_;}
 const Native::GxCommandState& state()const{return state_;}
};

