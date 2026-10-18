#pragma once
#include "Game/Util/Functor.hpp"
#include <memory>
#include <stdexcept>
#include <iostream>
inline void nativeFunctorTests(){
    struct Counter{mutable int calls=0;void run(){++calls;}void runConst()const{calls+=2;}};
    Counter counter;auto callback=MR::Functor(&counter,&Counter::run);callback();std::unique_ptr<MR::FunctorBase> copy(callback.clone(nullptr));(*copy)();if(counter.calls!=2)throw std::runtime_error("Native callback clone lost receiver");
    const Counter constCounter;auto constCallback=MR::Functor(&constCounter,&Counter::runConst);constCallback();std::unique_ptr<MR::FunctorBase> constCopy(constCallback.clone(nullptr));(*constCopy)();if(constCounter.calls!=4)throw std::runtime_error("Native const callback clone lost receiver");
    bool rejected=false;try{std::unique_ptr<MR::FunctorBase> unsupported(callback.clone(reinterpret_cast<JKRHeap*>(1)));}catch(const std::logic_error&){rejected=true;}if(!rejected)throw std::runtime_error("Unported heap was ignored");
    std::cout<<"PASS: native member callbacks, const dispatch, cloned receiver and explicit heap rejection\n";
}
