#include <Game/NameObj/NameObj.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
// Test-only observer for the external scene movement notification.
static int notifications=0;
namespace MR { void notifyRequestNameObjMovementOnOff(){++notifications;} }
struct Actor:NameObj{int steps=0;explicit Actor(const char* n):NameObj(n){}void movement()override{++steps;}};
int main(){SingletonHolder<NameObjRegister>::init();auto* registry=SingletonHolder<NameObjRegister>::get();NameObjHolder holder(32);registry->setCurrentHolder(&holder);
std::vector<std::string> names;names.reserve(24);for(int i=0;i<24;++i)names.push_back("actor"+std::to_string(i));
std::vector<std::unique_ptr<Actor>> actors;for(auto& name:names)actors.emplace_back(new Actor(name.c_str()));
for(int repeat=0;repeat<3;++repeat)for(int i=23;i>=0;--i)assert(holder.find(names[i].c_str())==actors[i].get());assert(holder.find("missing")==nullptr);
holder.callMethodAllObj(&NameObj::executeMovement);for(auto& actor:actors)assert(actor->steps==1);
holder.suspendAllObj();assert(notifications==24);holder.syncWithFlags();holder.callMethodAllObj(&NameObj::executeMovement);for(auto& actor:actors)assert(actor->steps==1);
holder.resumeAllObj();assert(notifications==48);holder.syncWithFlags();holder.callMethodAllObj(&NameObj::executeMovement);for(auto& actor:actors)assert(actor->steps==2);
holder.clearArray();assert(holder.find(names[0].c_str())==nullptr);holder.callMethodAllObj(&NameObj::executeMovement);for(auto& actor:actors)assert(actor->steps==2);registry->setCurrentHolder(nullptr);
std::puts("PASS original scene holder/registration: 24 actors, cache eviction, member dispatch, synchronized suspend/resume and clear");}