#pragma once
#include "Game/NameObj/NameObj.hpp"
#include "Game/NameObj/NameObjRegister.hpp"
#include "Game/Util/SingletonHolder.hpp"
inline size_t nativeMovementNotifications=0;
namespace MR { void notifyRequestNameObjMovementOnOff(){++nativeMovementNotifications;} }
struct NativeNameObjTest:NameObj {
    int movements=0,inits=0;
    NativeNameObjTest():NameObj("native NameObj test"){}
    void movement()override{++movements;}
    void init(const JMapInfoIter&)override{++inits;}
};
inline void nameObjTests(){
    SingletonHolder<NameObjRegister>::init();NameObjHolder holder;auto registry=SingletonHolder<NameObjRegister>::get();registry->setCurrentHolder(&holder);
    NativeNameObjTest object;assetCheck(holder.objects.size()==1&&holder.objects[0]==&object,"Original NameObj registration failed");
    object.initWithoutIter();object.executeMovement();assetCheck(object.inits==1&&object.movements==1,"Original NameObj virtual lifecycle failed");
    auto notifications=nativeMovementNotifications;NameObjFunction::requestMovementOff(&object);object.executeMovement();assetCheck(object.movements==2,"Suspend applied before synchronization");object.syncWithFlags();object.executeMovement();assetCheck(object.movements==2,"Original movement suspension failed");
    NameObjFunction::requestMovementOn(&object);object.syncWithFlags();object.executeMovement();assetCheck(object.movements==3&&nativeMovementNotifications==notifications+2,"Original movement resume/notification failed");
    object.requestSuspend();object.requestResume();object.syncWithFlags();object.executeMovement();assetCheck(object.movements==4,"Original movement request cancellation failed");registry->setCurrentHolder(nullptr);
}
