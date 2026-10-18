#include <JSystem/JUtility/JUTNameTab.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
int main(){
    // Host-order fixture; raw Wii BE bytes require conversion before this API.
    struct Resource {u16 count,pad;ResNTAB::Entry entry[2];char names[12];} resource{};
    JUTNameTab table;assert(table.mNameNum==0&&table.mResource==nullptr);
    resource.count=2;std::memcpy(resource.names,"Mario\0Luigi",12);
    resource.entry[0]={table.calcKeyCode("Mario"),12};resource.entry[1]={table.calcKeyCode("Luigi"),18};
    table.setResource(reinterpret_cast<const ResNTAB*>(&resource));
    assert(table.getIndex("Mario")==0&&table.getIndex("Luigi")==1&&table.getIndex("absent")==-1);
    assert(std::strcmp(table.getName(0),"Mario")==0&&std::strcmp(table.getName(1),"Luigi")==0&&table.getName(2)==nullptr);
    resource.entry[0].mKeyCode=table.calcKeyCode("Luigi");assert(table.getIndex("Luigi")==1);
    table.setResource(nullptr);assert(table.mNameNum==0&&table.mStrData==nullptr);
    std::puts("PASS original JUT name-table lookup and hash-collision name verification (host-order fixture)");
}

