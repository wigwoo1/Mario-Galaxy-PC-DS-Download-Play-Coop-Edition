#include "Game/Util/HashUtil.hpp"
#include <memory>
#include <limits>
#include <stdexcept>
#include <utility>
HashSortTable::HashSortTable(u32 count) {
    if(count>65535)throw std::length_error("HashSortTable exceeds u16 bucket domain");
    auto hashes=std::make_unique<u32[]>(count);
    auto values=std::make_unique<std::uintptr_t[]>(count);
    auto starts=std::make_unique<u16[]>(256);
    auto counts=std::make_unique<u16[]>(256);
    for(unsigned i=0;i<256;++i)starts[i]=0xffff;
    mMaxLength=count;mHashCodes=hashes.release();_8=values.release();_C=starts.release();_10=counts.release();
}
HashSortTable::~HashSortTable(){delete[] mHashCodes;delete[] _8;delete[] _C;delete[] _10;}
bool HashSortTable::addValue(u32 hash,std::uintptr_t value,bool skip) {
    if(skip)for(u32 i=0;i<mCurrentLength;++i)if(mHashCodes[i]==hash)return false;
    if(mCurrentLength==mMaxLength)return false;
    mHashCodes[mCurrentLength]=hash;_8[mCurrentLength]=value;++mCurrentLength;mHasBeenSorted=false;return true;
}
bool HashSortTable::add(const char* name,u32 value,bool skip){return addValue(MR::getHashCode(name),value,skip);}
bool HashSortTable::add(u32 hash,u32 value){return addValue(hash,value,false);}
bool HashSortTable::addOrSkip(u32 hash,u32 value){return addValue(hash,value,true);}
bool HashSortTable::addPointer(const char* name,const void* value,bool skip){return addValue(MR::getHashCode(name),reinterpret_cast<std::uintptr_t>(value),skip);}
void HashSortTable::sort() {
    // Original MR::sortSmall selection order, including duplicate-key swaps.
    for(u32 i=0;i<mCurrentLength;++i) {
        u32 smallest=i;
        for(u32 j=i+1;j<mCurrentLength;++j)if(mHashCodes[smallest]>mHashCodes[j])smallest=j;
        std::swap(mHashCodes[i],mHashCodes[smallest]);std::swap(_8[i],_8[smallest]);
    }
    for(unsigned i=0;i<256;++i){_C[i]=0xffff;_10[i]=0;}
    for(u32 i=0;i<mCurrentLength;++i){const u8 bucket=mHashCodes[i]>>24;if(_10[bucket]==0)_C[bucket]=static_cast<u16>(i);++_10[bucket];}
    mHasBeenSorted=true;
}
bool HashSortTable::searchValue(u32 hash,std::uintptr_t* result) {
    if(result)*result=0;
    if(!mHasBeenSorted)throw std::logic_error("HashSortTable search requires sort after mutation");
    const u8 bucket=hash>>24;
    for(u32 i=0;i<_10[bucket];++i){const u32 index=_C[bucket]+i;if(mHashCodes[index]==hash){if(result)*result=_8[index];return true;}if(mHashCodes[index]>hash)return false;}
    return false;
}
bool HashSortTable::search(u32 hash,u32* result) {
    if(result)*result=0;
    std::uintptr_t value=0;const bool found=searchValue(hash,&value);
    if(found&&result){if(value>std::numeric_limits<u32>::max())throw std::overflow_error("Pointer payload requested through integer hash lookup");*result=static_cast<u32>(value);}
    return found;
}
bool HashSortTable::search(const char* name,u32* result){return search(MR::getHashCode(name),result);}
bool HashSortTable::search(const char* first,const char* second,u32* result){return search(MR::getHashCode(first)+MR::getHashCode(second),result);}
bool HashSortTable::searchPointer(const char* name,void** result){if(result)*result=nullptr;std::uintptr_t value=0;const bool found=searchValue(MR::getHashCode(name),&value);if(found&&result)*result=reinterpret_cast<void*>(value);return found;}
void HashSortTable::swap(const char* first,const char* second){const u32 oldHash=MR::getHashCode(first);for(u32 i=0;i<mCurrentLength;++i)if(mHashCodes[i]==oldHash){mHashCodes[i]=MR::getHashCode(second);mHasBeenSorted=false;return;}}
