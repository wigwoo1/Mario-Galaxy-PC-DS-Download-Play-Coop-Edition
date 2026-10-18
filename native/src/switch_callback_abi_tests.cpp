#include <Game/Util/Functor.hpp>
#include <Game/Util/SwitchEventFunctorListener.hpp>
#include <cassert>
#include <cstdio>

struct Callback final : MR::FunctorBase {
    int* count;
    explicit Callback(int& value):count(&value){}
    void operator()() const override {++*count;}
    MR::FunctorBase* clone(JKRHeap* heap) const override {assert(!heap);return new Callback(*this);}
};
int main(){
    int on=0,off=0;
    SwitchEventFunctorListener listener;
    listener.listenSwitchOnEvent();listener.listenSwitchOffEvent();
    {
        Callback onSource(on),offSource(off);
        listener.setOnFunctor(onSource);listener.setOffFunctor(offSource);
        assert(listener.mOnFunctor!=&onSource&&listener.mOffFunctor!=&offSource);
    }
    SwitchEventListener* base=&listener;
    base->refresh(true);base->refresh(false);base->refresh(true);
    assert(on==2&&off==1);
    delete static_cast<Callback*>(listener.mOnFunctor);
    delete static_cast<Callback*>(listener.mOffFunctor);
    std::puts("PASS actual switch-listener provider clones and dispatches current-ABI callbacks after source lifetime");
}
