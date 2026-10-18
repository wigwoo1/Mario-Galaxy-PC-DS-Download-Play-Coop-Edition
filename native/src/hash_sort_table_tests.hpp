#pragma once
#include "Game/Util/HashUtil.hpp"
#include <stdexcept>
#include <limits>
#include <iostream>
inline void hashSortTableTests(){
    auto check=[](bool ok){if(!ok)throw std::runtime_error("native hash table regression");};
    HashSortTable empty(0);empty.sort();u32 value=123;check(!empty.search(u32{0},&value)&&value==0&&!empty.add(0,1));
    HashSortTable integers(5);check(integers.add(0xffffffff,71)&&integers.add(0,13)&&integers.add(2,20)&&integers.add(2,21)&&integers.add(1,14));check(!integers.add(7,3));integers.sort();check(integers.search(0xffffffff,&value)&&value==71);check(integers.search(u32{0},&value)&&value==13);check(integers.search(2,&value)&&value==20);check(!integers.search(3,&value)&&value==0);
    HashSortTable duplicateOrder(3);duplicateOrder.add(2,20);duplicateOrder.add(2,21);duplicateOrder.add(1,10);duplicateOrder.sort();check(duplicateOrder.search(2,&value)&&value==21);
    HashSortTable pointers(3);int payload=42;check(pointers.addPointer("entry",&payload,false));check(!pointers.addPointer("entry",nullptr,true));check(pointers.add("Aa",4,true));check(!pointers.add("BB",5,true));check(MR::getHashCode("Aa")==MR::getHashCode("BB"));pointers.sort();void* restored=nullptr;check(pointers.searchPointer("entry",&restored)&&restored==&payload&&*static_cast<int*>(restored)==42);check(!pointers.searchPointer("missing",&restored)&&restored==nullptr);
    if(reinterpret_cast<std::uintptr_t>(&payload)>std::numeric_limits<u32>::max()){bool rejected=false;try{pointers.search("entry",&value);}catch(const std::overflow_error&){rejected=true;}check(rejected&&value==0);}
    pointers.swap("entry","renamed");bool unsorted=false;try{pointers.searchPointer("renamed",&restored);}catch(const std::logic_error&){unsorted=true;}check(unsorted);pointers.sort();check(pointers.searchPointer("renamed",&restored)&&restored==&payload);check(!pointers.searchPointer("entry",&restored));
    HashSortTable combined(1);combined.add(MR::getHashCode("left")+MR::getHashCode("right"),9);combined.sort();check(combined.search("left","right",&value)&&value==9);
    bool tooLarge=false;try{HashSortTable invalid(65536);}catch(const std::length_error&){tooLarge=true;}check(tooLarge);
    std::cout<<"PASS: pointer-safe hash payloads, original duplicate order, buckets, indices and mutation guards\n";
}

