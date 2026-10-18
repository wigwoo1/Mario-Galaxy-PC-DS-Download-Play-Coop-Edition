#include "restart_cube.hpp"
namespace{GameDataTemporaryInGalaxy* restartTarget=nullptr;bool restartGrounded=false;}
NativeRestartScope::NativeRestartScope(GameDataTemporaryInGalaxy& target,bool grounded){assetCheck(!restartTarget&&target.mPlayerRestartIdInfo,"Invalid restart event scope");restartTarget=&target;restartGrounded=grounded;}
NativeRestartScope::~NativeRestartScope(){restartTarget=nullptr;}
namespace MR{
bool isOnGroundPlayer(){assetCheck(restartTarget,"Missing restart player scope");return restartGrounded;}
void setPlayerRestartIdInfo(const JMapIdInfo& id){assetCheck(restartTarget,"Missing restart state scope");restartTarget->setPlayerRestartIdInfo(id);}
bool isCubeBgmChangeInvalid(){return true;} // Audio service is not ported.
s32 getPlacedZoneId(const JMapInfoIter&);
}
// Native area base adapter: forms/followers/managers and switch services are supplied by the native scene.
AreaObj::AreaObj(int type,const char* name):NameObj(name),mForm(nullptr),mFormType(type),mIsValid(true),_15(true),mIsAwake(true),mObjArg0(-1),mObjArg1(-1),mObjArg2(-1),mObjArg3(-1),mObjArg4(-1),mObjArg5(-1),mObjArg6(-1),mObjArg7(-1),mSwitchCtrl(nullptr){assetCheck(type>=1&&type<=3,"Unsupported native area form");}
void AreaObj::init(const JMapInfoIter& iter){s32* args[]={&mObjArg0,&mObjArg1,&mObjArg2,&mObjArg3,&mObjArg4,&mObjArg5,&mObjArg6,&mObjArg7};for(size_t i=0;i<8;++i){auto field="Obj_arg"+std::to_string(i);*args[i]=-1;iter.getValue<s32>(field.c_str(),args[i]);}}

void RestartCube::changeBgm(){throw std::runtime_error("Restart BGM service is not ported");}
JMapIdInfo::JMapIdInfo(s32 id,const JMapInfoIter& iter){mZoneID=-1;initialize(id,iter);}
void JMapIdInfo::initialize(s32 id,const JMapInfoIter& iter){_0=id;mZoneID=MR::getPlacedZoneId(iter);}
RestartCube::RestartCube(int formType, const char* pName) : AreaObj(formType, pName), mIdInfo(), _40(-1), _44(-1), _48() {
}

void RestartCube::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);

    mIdInfo = new JMapIdInfo(mObjArg0, rIter);
    _40 = mObjArg1;
    _44 = mObjArg2;
    _48 = false;
}

void RestartCube::updatePlayerRestartIdInfo() {
    if (mObjArg3 == -1 || MR::isOnGroundPlayer()) {
        MR::setPlayerRestartIdInfo(*mIdInfo);

        if (!_48 && !MR::isCubeBgmChangeInvalid() && _40 >= 0) {
            changeBgm();
        }
    }
}

RestartCube::~RestartCube() {
}


