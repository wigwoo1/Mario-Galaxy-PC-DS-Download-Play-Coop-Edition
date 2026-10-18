#include <Game/Player/GroupChecker.hpp>
#include <Game/Util/HashUtil.hpp>
#include <Game/NameObj/NameObjHolder.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <cassert>
#include <cstdio>
#include <stdexcept>
int main(){
 NameObjHolder names(16);SingletonHolder<NameObjRegister>::init();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(&names);
 GroupCheckManager manager("groups");NameObj first("Turtle"),sameName("Turtle"),second("SpinningBox"),absent("Other");
 assert(manager._14==2&&manager.mGroups[0]->mHashTable->mMaxLength==32&&manager.mGroups[1]->mHashTable->mMaxLength==8);
 manager.add(&first,0);manager.add(&sameName,0);manager.add(&second,1);
 assert(manager.mGroups[0]->mHashTable->mCurrentLength==1&&manager.mGroups[1]->mHashTable->mCurrentLength==1);
 bool unsorted=false;try{manager.isExist(&first,0);}catch(const std::logic_error&){unsorted=true;}assert(unsorted);
 manager.mGroups[0]->initAfterPlacement();manager.mGroups[1]->initAfterPlacement();
 assert(manager.isExist(&first,0)&&manager.isExist(&sameName,0)&&!manager.isExist(&first,1));
 assert(manager.isExist(&second,1)&&!manager.isExist(&second,0)&&!manager.isExist(&absent,0));
 manager.add(&absent,0);manager.mGroups[0]->initAfterPlacement();assert(manager.isExist(&absent,0));
 for(auto* group:manager.mGroups){delete group->mHashTable;delete group;}
 names.clearArray();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(nullptr);
 std::puts("PASS original group checker: capacities, name membership, duplicate suppression, independent groups and placement sorting");
}
