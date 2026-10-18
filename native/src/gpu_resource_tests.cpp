#include "Native/GpuResources.hpp"
#include <cassert>
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <memory>
int main(){
    alignas(32) static const unsigned char imageTexture[64]={0x12,0x34,0x56,0x78};
    Native::registerExecutableImageResources(reinterpret_cast<const void*>(&main));
    auto imageAddress=Native::gpuResources().address(imageTexture);assert(Native::gpuResources().resolve(imageAddress,4)==imageTexture);
    auto imageChild=Native::gpuResources().add(imageTexture,sizeof(imageTexture));assert(imageChild!=imageAddress&&Native::gpuResources().byteAddress(imageTexture)==imageChild);
    bool imageWriteRejected=false;try{Native::gpuResources().addWritable(const_cast<unsigned char*>(imageTexture),sizeof(imageTexture));}catch(const std::invalid_argument&){imageWriteRejected=true;}assert(imageWriteRejected);
    Native::gpuResources().remove(imageTexture);assert(Native::gpuResources().byteAddress(imageTexture)==imageAddress);
    Native::GpuResources resources;unsigned char data[64]={7},other[32]={9};
    const auto address=resources.add(data,sizeof(data));assert(address%32==0&&resources.address(data+32)==address+32);
    data[0]=11;assert(resources.resolve(address,64)[0]==7);
    assert(resources.hostAddress(address,64)==data&&resources.hostAddress(address+7,4)==data+7);
    assert(resources.hostAddress(address,1)[0]==11&&resources.resolve(address,1)[0]==7);
    try{resources.hostAddress(address+32,33);assert(false);}catch(const std::out_of_range&){}
    bool overlapDetails=false;try{resources.add(data+1,8);assert(false);}catch(const std::invalid_argument& error){overlapDetails=std::strstr(error.what(),"request=")&&std::strstr(error.what(),"existing=")&&std::strstr(error.what(),"size=");}assert(overlapDetails);
    try{resources.address(data+1);assert(false);}catch(const std::invalid_argument&){}
    try{resources.resolve(address+32,33);assert(false);}catch(const std::out_of_range&){}
    resources.remove(data);try{resources.resolve(address,1);assert(false);}catch(const std::out_of_range&){}
    try{resources.hostAddress(address,1);assert(false);}catch(const std::out_of_range&){}
    const auto fresh=resources.add(other,32);assert(fresh>address&&resources.resolve(fresh,32)[0]==9);
    unsigned char target[10]={0,1,2,3,4,5,6,7,8,9};auto writable=resources.addWritable(target,10);
    unsigned char replacement[3]={21,22,23};auto generation=resources.generation();resources.write(writable+2,replacement,3);assert(resources.generation()==generation+1);
    assert(target[1]==1&&target[2]==21&&target[4]==23&&target[5]==5&&resources.resolve(writable+2,3)[1]==22);
    resources.write(writable+3,resources.resolve(writable+2,3),3);assert(target[3]==21&&target[5]==23);
    generation=resources.generation();try{resources.write(writable+9,replacement,3);assert(false);}catch(const std::out_of_range&){}
    assert(target[9]==9&&resources.generation()==generation);
    resources.write(writable,nullptr,0);assert(resources.generation()==generation);
    other[0]=17;resources.refresh(other,1);assert(resources.generation()==generation+1&&resources.resolve(fresh,1)[0]==17);
    try{resources.write(fresh,replacement,3);assert(false);}catch(const std::invalid_argument&){}
    assert(other[0]==17);
    resources.remove(target);try{resources.write(writable,replacement,1);assert(false);}catch(const std::out_of_range&){}
    auto owner=std::make_shared<std::vector<unsigned char>>(64,0x5a);std::weak_ptr<std::vector<unsigned char>> weak=owner;
    const void* borrowed=owner->data();auto borrowedAddress=resources.addBorrowed(borrowed,owner->size(),owner);
    owner.reset();assert(!weak.expired()&&resources.resolve(borrowedAddress,64)[0]==0x5a);
    assert(resources.hostAddress(borrowedAddress+4,60)==static_cast<const unsigned char*>(borrowed)+4);
    auto borrowedChild=resources.add(borrowed,16);assert(borrowedChild!=borrowedAddress&&resources.resolve(borrowedChild,16)[0]==0x5a);
    try{resources.addBorrowed(static_cast<const unsigned char*>(borrowed)+8,16,weak.lock());assert(false);}catch(const std::invalid_argument&){}
    try{resources.write(borrowedAddress,replacement,1);assert(false);}catch(const std::invalid_argument&){}
    auto retained=weak.lock();assert(retained);(*retained)[0]=0x6b;assert(resources.resolve(borrowedAddress,1)[0]==0x6b);
    retained.reset();resources.remove(borrowed);assert(!weak.expired());resources.remove(borrowed);assert(weak.expired());
    try{resources.resolve(borrowedAddress,1);assert(false);}catch(const std::out_of_range&){}
    auto writableOwner=std::make_shared<std::vector<unsigned char>>(96,0x3c);
    auto* live=writableOwner->data();
    const auto liveAddress=resources.addBorrowed(live,writableOwner->size(),writableOwner,true);
    generation=resources.generation();
    resources.write(liveAddress+16,replacement,3);
    assert(resources.generation()==generation+1);
    assert(live[15]==0x3c&&live[16]==21&&live[18]==23&&live[19]==0x3c);
    assert(resources.hostAddress(liveAddress+16,3)==live+16);
    assert(resources.resolve(liveAddress+16,3)==live+16);
    resources.write(liveAddress+17,resources.resolve(liveAddress+16,3),3);
    assert(live[17]==21&&live[18]==22&&live[19]==23&&live[20]==0x3c);
    generation=resources.generation();
    try{resources.write(liveAddress+95,replacement,3);assert(false);}catch(const std::out_of_range&){}
    assert(live[95]==0x3c&&resources.generation()==generation);
    resources.write(liveAddress,nullptr,0);assert(resources.generation()==generation);
    resources.remove(live);
    try{resources.resolve(liveAddress,1);assert(false);}catch(const std::out_of_range&){}
    std::puts("PASS GPU snapshot registration, bounded resolution, alignment, overlap rejection and stale address rejection");
    std::puts("PASS borrowed GPU range lifetime, live bytes, write protection, overlap checks and detach invalidation");
    std::puts("PASS writable borrowed GPU ranges, bounded live writes, aliasing and generation tracking");
    std::puts("PASS SDK host-address lookup preserves original storage instead of snapshot bytes, validates extents and rejects removed ranges");
}


