#pragma once
#include "native_model_blocks.hpp"
#include "native_joint_resource.hpp"
#include "native_envelope_resource.hpp"
#include "native_draw_matrix_resource.hpp"
#include "native_vertex_resource.hpp"
#include "native_shape_resource.hpp"
#include "native_material_init.hpp"
#include "native_model_material_flags.hpp"
#include "native_texture_tables.hpp"
#include "native_material_display_list.hpp"
#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/J3DGraphLoader/J3DModelLoader.hpp>
// Retained native sections shared by the original J3D objects; no source-file borrows.
struct NativeModelResource {
    std::unique_ptr<NativeJointResource> joints;
    std::unique_ptr<NativeEnvelopeResource> envelopes;
    std::unique_ptr<NativeDrawMatrixResource> drawMatrices;
    std::unique_ptr<NativeVertexResource> vertices;
    std::unique_ptr<NativeTextureTables> textureTables;
    std::unique_ptr<J3DTexture> textures;
    std::vector<std::unique_ptr<J3DMaterial>> materials;
    std::vector<J3DMaterial*> materialPointers;
    std::vector<std::unique_ptr<NativeMaterialDisplayList>> materialLists;
    std::unique_ptr<NativeShapeResource> shapes;
    std::vector<J3DModelHierarchy> hierarchyCommands;
    std::vector<u16> importantJoints;
    std::unique_ptr<NativeNameTable> materialNameStorage,textureNameStorage,shapeNameStorage;
    std::unique_ptr<JUTNameTab> materialNames,textureNames,shapeNames;
    std::unique_ptr<J3DMtxCalc> defaultCalculator;
    u32 informationFlags=0;
    u16 uniqueMaterialCount=0;
    J3DJointTree tree;
    bool texturesRegistered=false;
    explicit NativeModelResource(const std::vector<uint8_t>& bytes,u32 loaderFlags=0){
        NativeModelBlocks blocks(bytes.data(),bytes.size());
        const bool binaryDisplayLists=std::memcmp(bytes.data()+4,"bdl4",4)==0;tree.setModelDataType(binaryDisplayLists?1:0);
        auto find=[&](const char* tag){const NativeModelBlocks::Block* found=nullptr;
            for(auto& block:blocks.blocks)if(BinaryView(bytes,block.offset,block.length).tag(0)==tag){assetCheck(!found,"Duplicate J3D section");found=&block;}
            assetCheck(found,"Missing required J3D section");return BinaryView(bytes,found->offset,found->length);};
        joints=std::make_unique<NativeJointResource>(find("JNT1"));
        envelopes=std::make_unique<NativeEnvelopeResource>(find("EVP1"),joints->pointers.size());
        drawMatrices=std::make_unique<NativeDrawMatrixResource>(find("DRW1"),joints->pointers.size(),envelopes->counts.size());
        vertices=std::make_unique<NativeVertexResource>(find("VTX1"));
        for(auto block:blocks.blocks)if(BinaryView(bytes,block.offset,block.length).tag(0)=="TEX1")textureTables=std::make_unique<NativeTextureTables>(bytes.data()+block.offset,block.length);
        assetCheck(bool(textureTables),"Missing textures");textures=std::make_unique<J3DTexture>(textureTables->count,textureTables->headers());
        find("MAT3");
        // Material conversion only borrows during factory creation.
        for(auto block:blocks.blocks)if(BinaryView(bytes,block.offset,block.length).tag(0)=="MAT3"){
            NativeMaterialInitTables tables(bytes.data()+block.offset,block.length);J3DMaterialFactory factory(tables);
            // The original BDL reader uses createPatchedMaterial, which always
            // retains light-on color data, a full mutable PE/fog block, and
            // mutable TEV register storage independent of authored stage count.
            // Native display lists are rebuilt from MAT3; preserve those same
            // capabilities instead of selecting compact normal-material blocks.
            const u32 materialFlags=nativeModelMaterialFlags(binaryDisplayLists,loaderFlags);
            for(unsigned i=0;i<tables.header.materialCount;++i){materials.emplace_back(factory.createNormalMaterial(nullptr,int(i),materialFlags));assetCheck(bool(materials.back()),"Material creation failed");materialPointers.push_back(materials.back().get());}
            // Original readPatchedMaterial groups entries by their MAT3 remap ID.
            // Keep the pointer table stable before deriving its group namespace.
            uniqueMaterialCount=factory.countUniqueMaterials();
            const auto groupBase=reinterpret_cast<std::uintptr_t>(materialPointers.data())>>4;
            for(unsigned i=0;i<materialPointers.size();++i)
                materialPointers[i]->mDiffFlag=u32(groupBase+factory.getMaterialID(int(i)));
        }
        shapes=std::make_unique<NativeShapeResource>(find("SHP1"),loaderFlags);
        auto retainNames=[&](BinaryView section,unsigned field,size_t expected,std::unique_ptr<NativeNameTable>& storage,std::unique_ptr<JUTNameTab>& names){
            size_t offset=section.u32(field);if(!offset)return;
            assetCheck(offset>=field+4 && offset<section.size(),"Invalid model name table offset");
            std::vector<uint8_t> bytes(section.size()-offset);for(size_t i=0;i<bytes.size();++i)bytes[i]=section.u8(offset+i);
            storage=std::make_unique<NativeNameTable>(BinaryView(bytes));names=std::make_unique<JUTNameTab>(storage->resource());
            assetCheck(names->mNameNum==expected,"Model name table count mismatch");
        };
        retainNames(find("MAT3"),20,materials.size(),materialNameStorage,materialNames);
        retainNames(find("TEX1"),16,textureTables->count,textureNameStorage,textureNames);
        retainNames(find("SHP1"),20,shapes->pointers.size(),shapeNameStorage,shapeNames);
        auto hierarchy=find("INF1");vertices->data.mPacketNum=hierarchy.u32(12);vertices->data.mVtxNum=hierarchy.u32(16);joints->buildHierarchy(hierarchy);joints->attachMeshes(hierarchy,materialPointers,shapes->pointers);
        informationFlags=loaderFlags|hierarchy.u16(8);tree.setFlag(informationFlags);
        switch(informationFlags&15){
            case 0:defaultCalculator=std::make_unique<J3DMtxCalcNoAnm<J3DMtxCalcCalcTransformBasic,J3DMtxCalcJ3DSysInitBasic>>();break;
            case 1:defaultCalculator=std::make_unique<J3DMtxCalcNoAnm<J3DMtxCalcCalcTransformSoftimage,J3DMtxCalcJ3DSysInitSoftimage>>();break;
            case 2:defaultCalculator=std::make_unique<J3DMtxCalcNoAnm<J3DMtxCalcCalcTransformMaya,J3DMtxCalcJ3DSysInitMaya>>();break;
            default:assetCheck(false,"Unsupported original model matrix convention");
        }
        tree.setBasicMtxCalc(defaultCalculator.get());
        size_t hierarchyOffset=hierarchy.u32(20);bool terminated=false;
        for(size_t p=hierarchyOffset;p+4<=hierarchy.size();p+=4){hierarchyCommands.push_back({hierarchy.u16(p),hierarchy.u16(p+2)});if(!hierarchyCommands.back().mType){terminated=true;break;}}
        assetCheck(terminated,"Missing retained hierarchy terminator");
        joints->bind(tree);envelopes->bind(tree);tree.mDrawMtxData=drawMatrices->data;tree.setHierarchy(hierarchyCommands.data());
        assetCheck(tree.getDrawMtxNum()>=tree.getDrawFullWgtMtxNum()+tree.getWEvlpMtxNum(),"Draw table cannot hold important envelope indices");
        importantJoints.resize(tree.getDrawMtxNum());tree.mWEvlpImportantMtxIdx=importantJoints.data();tree.findImportantMtxIndex();
        for(auto* shape:shapes->pointers){shape->setVertexDataPointer(&vertices->data);shape->setDrawMtxDataPointer(&drawMatrices->data);}
    }
    // The resource must outlive this original model-data view.
    void bind(J3DModelData& model){
        model.mFlags=informationFlags;model.mJointTree=tree;model.mVertexData=vertices->data;
        model.mMaterialTable.mMaterialNum=u16(materialPointers.size());model.mMaterialTable.mUniqueMatNum=uniqueMaterialCount;
        model.mMaterialTable.mMaterialNodePointer=materialPointers.data();model.mMaterialTable.mTexture=textures.get();model.mMaterialTable.mMaterialName=materialNames.get();model.mMaterialTable.mTextureName=textureNames.get();
        model.mShapeTable.mShapeNum=u16(shapes->pointers.size());model.mShapeTable.mShapeNodePointer=shapes->pointers.data();model.mShapeTable.mShapeName=shapeNames.get();
        for(auto* shape:shapes->pointers){shape->setVertexDataPointer(&model.mVertexData);shape->setDrawMtxDataPointer(model.getDrawMtxData());}
    }
    void prepareMaterialLists(JKRHeap& heap,bool doubleBuffered=true){
        registerGpu();auto* previous=j3dSys.getTexture();j3dSys.setTexture(textures.get());
        try{materialLists.clear();for(auto* material:materialPointers){materialLists.emplace_back(std::make_unique<NativeMaterialDisplayList>(*material,heap));if(doubleBuffered)materialLists.back()->ensureDoubleBuffer();}}
        catch(...){j3dSys.setTexture(previous);throw;}j3dSys.setTexture(previous);
    }
    void registerGpu(){vertices->registerGpu();shapes->registerGeometry();if(!texturesRegistered){Native::gpuResources().add(textureTables->storage.data(),textureTables->storage.size());texturesRegistered=true;}}
    ~NativeModelResource(){if(texturesRegistered)Native::gpuResources().remove(textureTables->storage.data());}
    NativeModelResource(const NativeModelResource&)=delete;
    NativeModelResource& operator=(const NativeModelResource&)=delete;
};











