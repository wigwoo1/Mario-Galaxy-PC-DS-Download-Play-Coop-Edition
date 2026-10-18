#include <Game/Scene/StopSceneController.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/NameObj/NameObjGroup.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <cassert>
#include <cstdio>
static StopSceneController* active;static int registrations=0,calls=0;
namespace MR {
void notifyRequestNameObjMovementOnOff(){}
void connectToScene(NameObj*,int,int,int,int){++registrations;}
void stopScene(int frame){++calls;active->requestStopScene(frame);}
}
int main(){SingletonHolder<NameObjRegister>::init();auto* reg=SingletonHolder<NameObjRegister>::get();NameObjHolder holder(64);reg->setCurrentHolder(&holder);
 StopSceneController controller;active=&controller;assert(registrations==16&&!controller.isSceneStopped());
 controller.requestStopScene(5);controller.movement();assert(controller.mFrame==4);controller.requestStopScene(2);assert(controller.mFrame==4);controller.requestStopScene(8);assert(controller.mFrame==8);
 for(int i=0;i<8;++i)controller.movement();assert(!controller.isSceneStopped());controller.movement();assert(controller.mFrame==0);
 controller.requestStopSceneDelay(6,3);auto* first=static_cast<StopSceneDelayRequest*>(controller.mDelayRequestArray->getObj(0));for(int i=0;i<2;++i){first->movement();assert(!calls);}first->movement();assert(calls==1&&controller.mFrame==6&&!first->mDelay);first->movement();assert(calls==1);
 for(int i=0;i<16;++i)controller.requestStopSceneDelay(i+1,4);controller.requestStopSceneDelay(999,4);for(int i=0;i<16;++i){auto* req=static_cast<StopSceneDelayRequest*>(controller.mDelayRequestArray->getObj(i));assert(req->mFrame==i+1&&req->mDelay==4);for(int tick=0;tick<4;++tick)req->movement();}assert(calls==17&&controller.mFrame==16);
 for(int i=0;i<16;++i)delete controller.mDelayRequestArray->getObj(i);delete controller.mDelayRequestArray;holder.clearArray();reg->setCurrentHolder(nullptr);
 puts("PASS original scene stop: countdown, maximum-duration merge, exact delayed dispatch, 16-slot saturation and no repeated firing");
}
