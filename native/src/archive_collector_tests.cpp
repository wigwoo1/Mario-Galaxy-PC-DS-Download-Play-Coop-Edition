#include <Game/NameObj/NameObjArchiveListCollector.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <stdexcept>
int main(){
 NameObjArchiveListCollector collector;assert(collector.getArchiveNum()==0);
 std::string names[32];
 for(int i=0;i<32;++i){names[i]="Object"+std::to_string(i);collector.addArchive(names[i].c_str());}
 for(int i=0;i<32;++i)assert(names[i]==collector.getArchive(i));
 bool rejected=false;try{collector.addArchive("Overflow");}catch(const std::length_error&){rejected=true;}
 assert(rejected&&collector.getArchiveNum()==32);
 for(int i:{-1,32,1000}){rejected=false;try{collector.getArchive(i);}catch(const std::out_of_range&){rejected=true;}assert(rejected);}
 NameObjArchiveListCollector bounded;std::string maxName(63,'x');bounded.addArchive(maxName.c_str());
 assert(std::strlen(bounded.getArchive(0))==63);
 std::string tooLong(64,'x');rejected=false;try{bounded.addArchive(tooLong.c_str());}catch(const std::invalid_argument&){rejected=true;}
 assert(rejected&&bounded.getArchiveNum()==1&&maxName==bounded.getArchive(0));
 rejected=false;try{bounded.addArchive(nullptr);}catch(const std::invalid_argument&){rejected=true;}assert(rejected&&bounded.getArchiveNum()==1);
 std::puts("PASS original archive collector: owned names/order, 32-entry capacity, 63-byte name limit, rejected writes preserve state");
}
