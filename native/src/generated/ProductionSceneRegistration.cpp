#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/ObjUtil.hpp"
void MR::connectToScene(NameObj* pObj, int movementType, int calcAnimType, int drawBufferType, int drawType) {
    MR::registerNameObjToExecuteHolder(pObj, movementType, calcAnimType, drawBufferType, drawType);
    MR::connectToSceneTemporarily(pObj);
    MR::connectToDrawTemporarily(pObj);
}


