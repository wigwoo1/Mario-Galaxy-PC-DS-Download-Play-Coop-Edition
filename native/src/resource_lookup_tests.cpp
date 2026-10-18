#include "Game/System/ResourceInfo.hpp"
#include "Game/Util/HashUtil.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
int main(){
    assert(MR::getHashCodeLower("")==0);
    assert(MR::getHashCodeLower("ABC")==96354);
    assert(MR::getHashCodeLower("Mario_Run")==MR::getHashCodeLower("mARIO_rUN"));
    int a=1,b=2,c=3;ResTable table;table.newFileInfoTable(3);
    table.add("Mario_Run.bck",&a,true);table.add("Mario.Idle.bck",&b,true);table.add("Luigi.bdl",&c,false);
    assert(table.mCount==3&&table.getRes("mARIO_rUN")==&a);
    assert(table.getRes("MARIO.IDLE")==&b&&table.getRes("luigi.bdl")==&c);
    assert(table.getRes("Mario_Run.bck")==nullptr&&!table.isExistRes("absent"));
    assert(table.getResIndex("absent")==-1&&table.findFileInfo("absent")==nullptr);
    assert(table.getRes(static_cast<u32>(0))==&a&&table.getFileInfo(1)->mResource==&b);
    assert(std::strcmp(table.getResName(&a),"Mario_Run")==0&&table.getResName(&c)!=nullptr);
    assert(table.getResName(reinterpret_cast<void*>(1))==nullptr);
    for(unsigned i=0;i<table.mCount;++i)delete[] table.mFileInfoTable[i].mName;
    delete[] table.mFileInfoTable;
    std::puts("PASS original ResTable case-folded ASCII lookup, extension handling and reverse lookup");
}
