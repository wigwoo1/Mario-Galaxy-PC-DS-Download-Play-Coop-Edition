#include "JSystem/JKernel/JKRDvdRipper.hpp"
#include <type_traits>
template<class First> using NativeDvdLoad=void*(*)(First,u8*,JKRExpandSwitch,u32,JKRHeap*,JKRDvdRipper::EAllocDirection,u32,int*,u32*);
static_assert(std::is_same<decltype(static_cast<NativeDvdLoad<const char*>>(&JKRDvdRipper::loadToMainRAM)),NativeDvdLoad<const char*>>::value);
static_assert(std::is_same<decltype(static_cast<NativeDvdLoad<s32>>(&JKRDvdRipper::loadToMainRAM)),NativeDvdLoad<s32>>::value);
static_assert(std::is_same<decltype(static_cast<NativeDvdLoad<JKRDvdFile*>>(&JKRDvdRipper::loadToMainRAM)),NativeDvdLoad<JKRDvdFile*>>::value);
