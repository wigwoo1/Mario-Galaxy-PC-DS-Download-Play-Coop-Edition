#pragma once
#include "native_material_commands.hpp"
// Own an original material display list, including its GPU registration.
struct NativeMaterialDisplayList {
    JKRHeap* heap;
    void* storage=nullptr;
    void* secondStorage=nullptr;
    J3DDisplayListObj list;
    J3DMaterial* material;
    NativeMaterialDisplayList(J3DMaterial& input,JKRHeap& owner):heap(&owner),material(&input){
        NativeMaterialCommands commands(input,owner);size_t capacity=(commands.bytes().size()+31)&~size_t(31);
        storage=owner.alloc(u32(capacity),32);if(!storage)throw std::bad_alloc();
        try{std::memset(storage,0,capacity);std::memcpy(storage,commands.bytes().data(),commands.bytes().size());
            list.setSingleDisplayList(storage,u32(capacity));list.mSize=u32(commands.bytes().size());Native::gpuResources().add(storage,capacity);
            input.mSharedDLObj=&list;
        }catch(...){JKRHeap::free(storage,heap);storage=nullptr;throw;}
    }
    void ensureDoubleBuffer(){
        auto* previous=JKRHeap::sCurrentHeap;heap->becomeCurrentHeap();
        try{if(list.single_To_Double()!=0)throw std::runtime_error("Material double buffer allocation failed");}
        catch(...){if(previous)previous->becomeCurrentHeap();else JKRHeap::sCurrentHeap=nullptr;throw;}
        if(previous)previous->becomeCurrentHeap();else JKRHeap::sCurrentHeap=nullptr;
        void* other=list.mpDisplayList[0]==storage?list.mpDisplayList[1]:list.mpDisplayList[0];
        if(other!=storage && !secondStorage){secondStorage=other;Native::gpuResources().add(secondStorage,list.mMaxSize);}
    }
    ~NativeMaterialDisplayList(){if(material->mSharedDLObj==&list)material->mSharedDLObj=nullptr;if(secondStorage){Native::gpuResources().remove(secondStorage);JKRHeap::free(secondStorage,heap);}if(storage){Native::gpuResources().remove(storage);JKRHeap::free(storage,heap);}}
    NativeMaterialDisplayList(const NativeMaterialDisplayList&)=delete;
    NativeMaterialDisplayList& operator=(const NativeMaterialDisplayList&)=delete;
};

