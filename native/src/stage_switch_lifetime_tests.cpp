#include <Game/Map/StageSwitch.hpp>
#include <Game/NameObj/NameObjHolder.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/Util/JMapIdInfo.hpp>
#include <cstdlib>
#include <new>
#include <cassert>
#include <cstdio>
static void* tracked[32]{};static int live=0,allocations=0;
void* operator new(std::size_t size){auto* p=std::malloc(size);if(!p)throw std::bad_alloc();if(size==sizeof(ZoneSwitch)){int slot=0;while(slot<32&&tracked[slot])++slot;assert(slot<32);tracked[slot]=p;++live;++allocations;}return p;}
void operator delete(void* p) noexcept{if(p)for(auto& entry:tracked)if(entry==p){entry=nullptr;--live;break;}std::free(p);}
void operator delete(void* p,std::size_t) noexcept{::operator delete(p);}
static int fixtureZone=0;
namespace MR {s32 getPlacedZoneId(const JMapInfoIter&){return fixtureZone;}}
int main(){NameObjHolder names(8);SingletonHolder<NameObjRegister>::init();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(&names);
for(int i=0;i<100;i++){{StageSwitchContainer container;assert(live==1);for(int zone=0;zone<3;zone++){
fixtureZone=zone;SwitchIdInfo info(31,JMapInfoIter());container.createAndAddZone(info);auto* storage=container.getZoneSwitch(info);assert(storage&&!storage->get(31));storage->set(31,true);
const int prior=allocations;container.createAndAddZone(info);assert(allocations==prior&&container.getZoneSwitch(info)==storage);delete info.mIDInfo;
}assert(live==4);fixtureZone=1;SwitchIdInfo global(1031,JMapInfoIter());assert(global.getSwitchNo()==31&&!container.getZoneSwitch(global)->get(31));delete global.mIDInfo;
}assert(live==0);names.clearArray();}
SingletonHolder<NameObjRegister>::get()->setCurrentHolder(nullptr);std::puts("PASS 100 stage-switch lifetimes: global/zone isolation, duplicate zone reuse and all storage released");}
