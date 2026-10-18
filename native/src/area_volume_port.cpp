#include "area_volume.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/Map/StageSwitch.hpp"
#include <map>
namespace{std::map<const AreaObj*,std::function<bool(const TVec3f&)>> volumes;}
void bindNativeAreaVolume(const AreaObj* area,std::function<bool(const TVec3f&)> contains){assetCheck(area&&contains&&volumes.emplace(area,std::move(contains)).second,"Invalid/duplicate native area volume");}
void unbindNativeAreaVolume(const AreaObj* area){volumes.erase(area);}
bool AreaObj::isInVolume(const TVec3f& p)const{if(!isValid())return false;auto found=volumes.find(this);assetCheck(found!=volumes.end(),"Unbound native area geometry");return found->second(p);}
bool AreaObj::isValidSwitchA()const{return mSwitchCtrl&&mSwitchCtrl->isValidSwitchA();}
bool AreaObj::isOnSwitchA()const{return mSwitchCtrl&&mSwitchCtrl->isOnSwitchA();}

