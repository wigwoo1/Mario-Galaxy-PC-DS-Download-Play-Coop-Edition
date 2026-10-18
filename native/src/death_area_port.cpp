#include "death_area.hpp"
namespace{NativeFatalAreaEvent* fatalEvent=nullptr;}
NativeFatalAreaScope::NativeFatalAreaScope(NativeFatalAreaEvent& event){assetCheck(!fatalEvent,"Nested native fatal-area scope");fatalEvent=&event;}
NativeFatalAreaScope::~NativeFatalAreaScope(){fatalEvent=nullptr;}
namespace MR{
void connectToSceneAreaObj(AreaObj*){} // NativeScene registers original NameObj lifetime/phases.
const TVec3f* getPlayerPos(){assetCheck(fatalEvent,"Missing fatal-area player scope");return &fatalEvent->position;}
void forceKillPlayerByAbyss(){assetCheck(fatalEvent,"Missing fatal-area event scope");fatalEvent->requested=true;}
}
DeathArea::DeathArea(int formType, const char* pName) : AreaObj(formType, pName) {
}

void DeathArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
}

void DeathArea::movement() {
    if (!isInVolume(*MR::getPlayerPos())) {
        return;
    }

    bool canKillPlayer = getDeathType() == DeathType_Any;

    if (!canKillPlayer) {
        return;
    }

    MR::forceKillPlayerByAbyss();
}

bool DeathArea::isInVolume(const TVec3f& rVec) const {
    if (isValidSwitchA() && !isOnSwitchA()) {
        return false;
    }

    return AreaObj::isInVolume(rVec);
}

s32 DeathArea::getDeathType() const {
    if (mObjArg0 != -1) {
        return mObjArg0;
    }

    return DeathType_Any;
}

