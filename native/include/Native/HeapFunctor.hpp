#pragma once
#include <Game/Util/Functor.hpp>
#include <Native/HeapPlacement.hpp>
namespace MR {
struct alignas(std::max_align_t) NativeCallbackAllocation {JKRHeap* heap;};
template<class Callback> class NativeHeapCallback final:public FunctorBase {
 static_assert(alignof(Callback)<=alignof(NativeCallbackAllocation),"Over-aligned native callbacks require a larger allocation header");
 Callback callback;
public:
 explicit NativeHeapCallback(const Callback& value):callback(value){}
 void operator()()const override{callback();}
 FunctorBase* clone(JKRHeap* heap)const override{return new(heap) NativeHeapCallback(callback);}
 bool dispose()noexcept override{this->~NativeHeapCallback();operator delete(this);return true;}
 static void* operator new(std::size_t size,JKRHeap* heap){
  if(size>std::numeric_limits<u32>::max()-sizeof(NativeCallbackAllocation))throw std::bad_alloc();
  const std::size_t allocationSize=size+sizeof(NativeCallbackAllocation);
  void* storage=heap
      ? JKRHeap::alloc(static_cast<u32>(allocationSize),alignof(NativeCallbackAllocation),heap)
      : ::operator new(allocationSize);
  if(!storage)throw std::bad_alloc();
  auto header=::new(storage) NativeCallbackAllocation{heap};return header+1;
 }
 static void operator delete(void* pointer)noexcept{if(!pointer)return;auto header=static_cast<NativeCallbackAllocation*>(pointer)-1;auto heap=header->heap;header->~NativeCallbackAllocation();if(heap)JKRHeap::free(header,heap);else ::operator delete(header);}
 static void operator delete(void* pointer,JKRHeap*)noexcept{operator delete(pointer);}
};
template<class Callback> FunctorBase* nativeCloneHeapCallback(const Callback& callback,JKRHeap* heap){return new(heap) NativeHeapCallback<Callback>(callback);}
}



