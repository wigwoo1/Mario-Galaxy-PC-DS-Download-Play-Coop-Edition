#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <Native/AudioPoolSize.hpp>
#include <type_traits>
#include <cstdio>
struct AudioPoolProbe { u32 value; };
using Single=JASPoolAllocObject<AudioPoolProbe>;
using Multi=JASPoolAllocObject_MultiThreaded<AudioPoolProbe>;
static_assert(std::is_same<decltype(&Single::operator new),void*(*)(std::size_t)>::value);
static_assert(std::is_same<decltype(&Multi::operator new),void*(*)(std::size_t)>::value);
static_assert(std::is_same<decltype(&Single::operator delete),void(*)(void*,std::size_t) noexcept>::value);
static_assert(std::is_same<decltype(&Multi::operator delete),void(*)(void*,std::size_t) noexcept>::value);
int main(){if(nativeAudioPoolSize(0)!=0||nativeAudioPoolSize(64)!=64||nativeAudioPoolSize(std::numeric_limits<u32>::max())!=std::numeric_limits<u32>::max())return 1;bool rejected=false;try{nativeAudioPoolSize(std::size_t(std::numeric_limits<u32>::max())+1);}catch(const std::bad_alloc&){rejected=true;}if(!rejected)return 2;std::puts("PASS actual audio allocator signatures and checked native-to-u32 size boundaries");}

