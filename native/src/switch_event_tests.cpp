#include <Game/Util/SwitchEventFunctorListener.hpp>
#include <Game/Util/Functor.hpp>
#include <cassert>
#include <cstdio>
struct Callback : MR::FunctorBase {
    int* calls;Callback* copy;
    explicit Callback(int& count):calls(&count),copy(nullptr){}
    void operator()() const override {++*calls;}
    MR::FunctorBase* clone(JKRHeap* heap) const override {
        assert(heap==nullptr && copy);return copy;
    }
};
int main(){
    SwitchEventFunctorListener listener;
    assert(!listener.mOnFunctor && !listener.mOffFunctor);
    listener.refresh(true);listener.refresh(false);
    int on=0,off=0;
    Callback onCopy(on),offCopy(off),onSource(on),offSource(off);
    onSource.copy=&onCopy;offSource.copy=&offCopy;
    listener.setOnFunctor(onSource);listener.setOffFunctor(offSource);
    assert(listener.mOnFunctor==&onCopy && listener.mOffFunctor==&offCopy);
    listener.refresh(true);assert(on==1 && off==0);
    listener.refresh(false);assert(on==1 && off==1);
    listener.listenSwitchOnEvent();assert(on==2 && off==1);
    std::puts("Original switch-event clone routing and callback dispatch passed");
}
