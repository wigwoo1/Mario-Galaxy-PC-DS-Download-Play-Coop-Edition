#include <cassert>
#include <cstring>
#include <cstdio>
namespace MR {const char* getBasename(const char*);bool isEqualString(const char*,const char*);}
int main(){
    const char* paths[]={"","Mario","ObjectData/Mario.arc","/StageData/EggPlanet/","/","a//b","a\\b"};
    const char* expected[]={"","Mario","Mario.arc","","","b","a\\b"};
    for(unsigned i=0;i<7;++i){auto base=MR::getBasename(paths[i]);assert(!std::strcmp(base,expected[i]));assert(base>=paths[i] && base<=paths[i]+std::strlen(paths[i]));}
    assert(MR::isEqualString("", ""));assert(MR::isEqualString("Mario", "Mario"));
    assert(!MR::isEqualString("Mario", "mario"));assert(!MR::isEqualString("Mario", "Mario.arc"));
    std::puts("PASS original basename borrowed-pointer behavior and exact case-sensitive string equality");
}

