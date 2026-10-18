#include "MarioActorAccess.hpp"
#include <Game/Player/MarioHolder.hpp>
#include <Game/Player/MarioAccess.hpp>
#include <Game/Util/PlayerUtil.hpp>
namespace MarioAccess {
    MarioActor* getPlayerActor() {
        return MR::getMarioHolder()->getMarioActor();
    }
}
namespace MR {
    TVec3f* getPlayerPos() {
        return &MarioAccess::getPlayerActor()->mPosition;
    }
}
