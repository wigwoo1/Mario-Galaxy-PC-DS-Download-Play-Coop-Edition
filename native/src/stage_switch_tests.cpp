#include <Game/Map/StageSwitch.hpp>
#include <cassert>
#include <cstdio>
int main(){
    ZoneSwitch first,second;
    for(int bit=0;bit<128;++bit)assert(!first.get(bit) && !second.get(bit));
    for(int bit=0;bit<128;++bit){
        first.set(bit,true);
        for(int other=0;other<128;++other){
            assert(first.get(other)==(other==bit));
            assert(!second.get(other));
        }
        first.set(bit,false);assert(!first.get(bit));
    }
    for(int bit=0;bit<128;++bit)first.set(bit,true);
    first.set(31,false);first.set(32,false);first.set(127,false);
    for(int bit=0;bit<128;++bit)assert(first.get(bit)==(bit!=31 && bit!=32 && bit!=127));
    std::puts("Original stage switch: all 128 bits, boundary clearing and zone isolation passed");
}
