#pragma once
#include "native_shape_tables.hpp"
#include <Native/GpuResources.hpp>
#include <JSystem/J3DGraphBase/J3DShape.hpp>
#include <JSystem/J3DGraphBase/J3DShapeDraw.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
struct NativeShapeDelete {
    void operator()(J3DShape* shape) const{if(!shape)return;
        for(unsigned group=0;group<shape->mMtxGroupNum;++group){delete shape->mShapeMtx[group];delete shape->mShapeDraw[group];}
        delete[] shape->mShapeMtx;delete[] shape->mShapeDraw;delete shape;
    }
};
// Own all data borrowed by original shape matrix/draw objects and VCD commands.
struct NativeShapeResource {
    NativeShapeTables tables;
    struct CommandDelete{JKRHeap* heap;void operator()(u8* data)const{if(data)JKRHeap::free(data,heap);}};
    std::unique_ptr<u8,CommandDelete> commands;
    std::vector<std::unique_ptr<J3DShape,NativeShapeDelete>> nodes;
    std::vector<J3DShape*> pointers;
    explicit NativeShapeResource(BinaryView block,u32 flags=0):tables(block),commands(nullptr,CommandDelete{JKRHeap::sCurrentHeap}){
        tables.decodeGeometryTables(block);J3DShapeBlock empty{};J3DShapeFactory factory(empty);tables.bindGeometry(factory);
        factory.allocVcdVatCmdBuffer(u32(tables.indices.size()));commands.reset(factory.mVcdVatCmdBuffer);
        nodes.reserve(tables.indices.size());pointers.reserve(tables.indices.size());
        for(size_t i=0;i<tables.indices.size();++i){nodes.emplace_back(factory.create(int(i),flags,nullptr));pointers.push_back(nodes.back().get());}
    }
    struct RewrittenList{void* pointer;JKRHeap* heap;};
    std::vector<RewrittenList> rewrittenLists;
    std::vector<std::unique_ptr<GXVtxDescList[]>> rewrittenDescriptors;
    void retainRewrittenDescriptors(){for(auto* shape:pointers){auto* pointer=shape->getVtxDesc();bool original=false;for(auto& record:tables.records)if(pointer==tables.descriptors.data()+record.mVtxDescListIndex/sizeof(GXVtxDescList)){original=true;break;}if(original)continue;bool retained=false;for(auto& entry:rewrittenDescriptors)if(entry.get()==pointer){retained=true;break;}if(!retained)rewrittenDescriptors.emplace_back(pointer);}}
    std::vector<std::unique_ptr<u8,CommandDelete>> privateCommands;
    void makeCommandPrivate(J3DShape& shape,JKRHeap& heap){
        bool owned=false;for(auto* entry:pointers)if(entry==&shape){owned=true;break;}assetCheck(owned,"Cannot replace commands of a foreign shape");
        auto* storage=static_cast<u8*>(heap.alloc(J3DShape::kVcdVatDLSize,32));assetCheck(storage!=nullptr,"Shape command allocation failed");
        std::unique_ptr<u8,CommandDelete> owner(storage,CommandDelete{&heap});
        std::memcpy(storage,shape.getVcdVatCmd(),J3DShape::kVcdVatDLSize);
        privateCommands.push_back(std::move(owner));Native::gpuResources().add(storage,J3DShape::kVcdVatDLSize);shape.setVcdVatCmd(storage);
    }
    bool geometryRegistered=false;
    void registerGeometry(){if(!geometryRegistered){Native::gpuResources().add(tables.displayLists.data(),tables.displayLists.size());geometryRegistered=true;}}
    bool commandsRegistered=false;
    void prepareVertexCommands(J3DVertexData& vertices){
        for(auto* shape:pointers){shape->setVertexDataPointer(&vertices);shape->makeVcdVatCmd();}
        const size_t size=pointers.size()*J3DShape::kVcdVatDLSize;
        if(!size)return;
        for(auto& entry:privateCommands)Native::gpuResources().refresh(entry.get(),J3DShape::kVcdVatDLSize);
        if(commandsRegistered)Native::gpuResources().refresh(commands.get(),size);
        else{Native::gpuResources().add(commands.get(),size);commandsRegistered=true;}
    }
    void retainRewrittenGeometry(){
        for(auto* shape:pointers)for(unsigned group=0;group<shape->mMtxGroupNum;++group){auto* draw=shape->mShapeDraw[group];auto* pointer=draw->getDisplayList();
            bool original=false;for(auto& record:tables.drawRecords)if(pointer==tables.displayLists.data()+record.mDisplayListIndex){original=true;break;}if(original)continue;
            bool retained=false;for(auto record:rewrittenLists)if(record.pointer==pointer){retained=true;break;}if(retained)continue;
            auto* heap=JKRHeap::findFromRoot(pointer);assetCheck(heap!=nullptr,"Rewritten geometry has no owning heap");
            rewrittenLists.push_back({pointer,heap});Native::gpuResources().add(pointer,draw->getDisplayListSize());
        }
    }
    ~NativeShapeResource(){for(auto& entry:privateCommands)Native::gpuResources().remove(entry.get());for(auto record:rewrittenLists){Native::gpuResources().remove(record.pointer);JKRHeap::free(record.pointer,record.heap);}if(commandsRegistered)Native::gpuResources().remove(commands.get());if(geometryRegistered)Native::gpuResources().remove(tables.displayLists.data());}
    NativeShapeResource(const NativeShapeResource&)=delete;
    NativeShapeResource& operator=(const NativeShapeResource&)=delete;
};





