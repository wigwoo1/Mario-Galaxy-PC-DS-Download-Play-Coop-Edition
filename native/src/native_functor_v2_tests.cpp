#include <Game/Util/Functor.hpp>
#include <cassert>
#include <memory>
#include <cstdio>
struct Receiver {int value=0;void apply(int add,int* result){value+=add;*result=value;}void observe(int add,int* result)const{*result=value+add;}};
int main(){Receiver receiver;int result=0;auto callback=MR::Functor(&receiver,&Receiver::apply,7,&result);callback();assert(receiver.value==7 && result==7);std::unique_ptr<MR::FunctorBase> copy(callback.clone(nullptr));copy->operator()();assert(receiver.value==14 && result==14);const Receiver& observed=receiver;auto constant=MR::Functor(&observed,&Receiver::observe,3,&result);std::unique_ptr<MR::FunctorBase> constCopy(constant.clone(nullptr));constCopy->operator()();assert(receiver.value==14 && result==17);std::puts("PASS two-argument native member callbacks");}
