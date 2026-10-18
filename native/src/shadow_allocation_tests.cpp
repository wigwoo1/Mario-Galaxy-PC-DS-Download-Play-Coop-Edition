#include <JSystem/JGeometry/TVec.hpp>
#include <new>
#include <cstdint>
#include <type_traits>
#include <cstdio>
static_assert(sizeof(TVec3f)==12&&sizeof(TVec2f)==8&&sizeof(Mtx)==48);
static_assert(std::is_trivially_destructible<TVec3f>::value&&std::is_trivially_destructible<TVec2f>::value);
template<class T> bool checkArray(unsigned count){
    T* p=new(std::align_val_t{32}) T[count];
    bool aligned=reinterpret_cast<std::uintptr_t>(p)%32==0;
    auto* bytes=reinterpret_cast<unsigned char*>(p);
    for(unsigned i=0;i<count*sizeof(T);++i)bytes[i]=static_cast<unsigned char>(i);
    bool intact=true;for(unsigned i=0;i<count*sizeof(T);++i)if(bytes[i]!=static_cast<unsigned char>(i))intact=false;
    ::operator delete[](p,std::align_val_t{32});return aligned&&intact;
}
int main(){for(unsigned count=1;count<=65536;count*=2)if(!checkArray<TVec3f>(count)||!checkArray<TVec2f>(count)||!checkArray<u8>(count)||!checkArray<Mtx>(count))return 1;if(!checkArray<u8>(0x20000))return 2;std::puts("PASS actual shadow vector/byte aligned arrays, strides, full storage and matching release");}


