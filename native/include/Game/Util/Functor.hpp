#pragma once
// Native member callbacks with optional JKR-owned cloning.
class JKRHeap;
namespace MR {
// Native callback ABI extends the game's callback contract without changing
// the order of its original call and clone virtual slots.
class FunctorBase {
public:
    virtual void operator()() const = 0;
    virtual FunctorBase* clone(JKRHeap*) const = 0;
    // Preserve the game's original operator()/clone() slots. Heap-owned native
    // clones opt into disposal; legacy callback implementations can use the
    // JKRHeap fallback without a vtable layout change to existing methods.
    virtual bool dispose() noexcept { return false; }
};
}
#include <stdexcept>
namespace MR {template<class Callback> FunctorBase* nativeCloneHeapCallback(const Callback&,JKRHeap*);}
namespace MR {
template<class Caller,class Method> class FunctorV0M:public FunctorBase {
public:
    FunctorV0M(Caller caller,Method method):mCaller(caller),mCallee(method){}
    void operator()()const override{(mCaller->*mCallee)();}
    FunctorBase* clone(JKRHeap* heap)const override{if(heap)return nativeCloneHeapCallback(*this,heap);return new FunctorV0M(*this);}
    Caller mCaller;Method mCallee;
};
template<class T> FunctorV0M<T*,void(T::*)()> Functor(T* caller,void(T::*method)()){return {caller,method};}
template<class T> FunctorV0M<const T*,void(T::*)()const> Functor(const T* caller,void(T::*method)()const){return {caller,method};}
}

namespace MR {
template<class Caller,class Method,class A,class B> class FunctorV2M:public FunctorBase {
public:
 FunctorV2M(Caller caller,Method method,A first,B second):mCaller(caller),mCallee(method),mArg0(first),mArg1(second){}
 void operator()()const override{(mCaller->*mCallee)(mArg0,mArg1);}
 FunctorBase* clone(JKRHeap* heap)const override{if(heap)return nativeCloneHeapCallback(*this,heap);return new FunctorV2M(*this);}
 Caller mCaller;Method mCallee;A mArg0;B mArg1;
};
template<class T,class A,class B> FunctorV2M<T*,void(T::*)(A,B),A,B> Functor(T* caller,void(T::*method)(A,B),A first,B second){return {caller,method,first,second};}
template<class T,class A,class B> FunctorV2M<const T*,void(T::*)(A,B)const,A,B> Functor(const T* caller,void(T::*method)(A,B)const,A first,B second){return {caller,method,first,second};}
}

#include <Native/HeapFunctor.hpp>

namespace MR {
class FunctorV0F:public FunctorBase {
public:
 explicit FunctorV0F(void (*function)()):mFunc(function){}
 void operator()()const override{mFunc();}
 FunctorBase* clone(JKRHeap* heap)const override{if(heap)return nativeCloneHeapCallback(*this,heap);return new FunctorV0F(*this);}
 void (*mFunc)();
};
inline FunctorV0F Functor(void (*function)()){return FunctorV0F(function);}
}

namespace MR {
template<class Caller,class Method,class A> class FunctorV1M:public FunctorBase {
public:
 FunctorV1M(Caller caller,Method method,A argument):mCaller(caller),mCallee(method),mArg0(argument){}
 void operator()()const override{(mCaller->*mCallee)(mArg0);}
 FunctorBase* clone(JKRHeap* heap)const override{if(heap)return nativeCloneHeapCallback(*this,heap);return new FunctorV1M(*this);}
 Caller mCaller;Method mCallee;A mArg0;
};
template<class T,class A> FunctorV1M<T*,void(T::*)(A),A> Functor(T* caller,void(T::*method)(A),A argument){return {caller,method,argument};}
template<class T,class A> FunctorV1M<const T*,void(T::*)(A)const,A> Functor(const T* caller,void(T::*method)(A)const,A argument){return {caller,method,argument};}
}
