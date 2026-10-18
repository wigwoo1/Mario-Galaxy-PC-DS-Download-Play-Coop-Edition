#pragma once
#include <Game/Util/JMapInfo.hpp>
#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRDisposer.hpp>
#include <stdexcept>
namespace Native {
class HeapJMapOwner final:public JKRDisposer {
 JMapInfo table;
public:
 explicit HeapJMapOwner(const JMapInfo& source):table(source){}
};
inline void retainHeapJMap(const JMapInfo& table,JKRHeap* heap){
 if(!heap||!table.mNativeStorage)throw std::invalid_argument("Camera table has no native heap owner");
 new(heap,0) HeapJMapOwner(table);
}
}
