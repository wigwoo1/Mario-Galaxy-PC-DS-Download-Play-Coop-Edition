#pragma once
#include "native_model_material_flags.hpp"
#include "native_tev_reg_animation.hpp"
#include <JSystem/J3DGraphAnimator/J3DMaterialAttach.hpp>
#include <JSystem/J3DGraphAnimator/J3DMaterialAnm.hpp>

inline void verifyRetailBrkMaterials(WiiAssets& disc,JKRHeap& heap){
    for(u32 flags:{0u,u32(J3DMLF_18|J3DMLF_Material_UseIndirect|J3DMLF_UseUniqueMaterials)}){
        assert(nativeModelMaterialFlags(false,flags)==flags);
        const u32 selected=nativeModelMaterialFlags(true,flags);
        assert(getMdlDataFlag_TevStageNum(selected)==16);
        assert((selected&~(0xf0000000u|0x001f0000u))==(flags&~(0xf0000000u|0x001f0000u)));
    }
    size_t resources=0,updates=0,oneStage=0;
    for(const char* path:{"/ObjectData/Butterfly.arc","/ObjectData/Kinopio.arc"}){
        Rarc archive(disc.readFile(path));
        for(const auto& file:archive.files){
            auto model=archive.readFile(file.path);
            if(model.size()<32 || std::memcmp(model.data(),"J3D2",4))continue;
            const bool bdl=std::memcmp(model.data()+4,"bdl4",4)==0;
            NativeModelBlocks blocks(model.data(),model.size());
            std::unique_ptr<NativeTextureTables> textures;
            for(const auto& b:blocks.blocks)if(!std::memcmp(model.data()+b.offset,"TEX1",4))textures=std::make_unique<NativeTextureTables>(model.data()+b.offset,b.length);
            assert(textures);J3DTexture texture(textures->count,textures->headers());auto* previous=j3dSys.getTexture();j3dSys.setTexture(&texture);
            Native::gpuResources().add(textures->storage.data(),textures->storage.size());
            for(const auto& b:blocks.blocks){
                if(std::memcmp(model.data()+b.offset,"MAT3",4))continue;
                NativeMaterialInitTables tables(model.data()+b.offset,b.length);J3DMaterialFactory factory(tables);
                BinaryView mat(model,b.offset,b.length);auto nameOffset=mat.u32(20);NativeNameTable nameStorage(mat.sub(nameOffset,mat.size()-nameOffset));JUTNameTab names(nameStorage.resource());
                const auto flags=nativeModelMaterialFlags(bdl,bdl?u32(J3DMLF_Material_UseIndirect|J3DMLF_UseUniqueMaterials):u32(J3DMLF_Material_PE_Full|J3DMLF_Material_UseIndirect|J3DMLF_UseUniqueMaterials|J3DMLF_21));
                std::vector<J3DMaterial*> materials;std::vector<std::unique_ptr<J3DMaterialAnm>> animators;J3DShape shape;
                for(unsigned i=0;i<tables.header.materialCount;++i){
                    auto* m=factory.createNormalMaterial(nullptr,int(i),flags);assert(m && m->mTevBlock);
                    assert(m->mTevBlock->getTevStageNum()==factory.newTevStageNum(i));
                    if(m->mTevBlock->getTevStageNum()==1)++oneStage;
                    animators.emplace_back(new J3DMaterialAnm());m->mMaterialAnm=animators.back().get();m->mShape=&shape;materials.push_back(m);
                }
                J3DMaterialTable table;table.mMaterialNum=u16(materials.size());table.mMaterialNodePointer=materials.data();table.mMaterialName=&names;
                for(const auto& af:archive.files){
                    auto source=archive.readFile(af.path);if(source.size()<8 || std::memcmp(source.data(),"J3D1brk1",8))continue;
                    NativeTevRegAnimation animation(source.data(),source.size());std::fill(source.begin(),source.end(),0xdd);animation.searchUpdateMaterialID(&table);
                    for(unsigned i=0;i<animation.mCRegUpdateMaterialNum;++i)if(animation.isValidCRegUpdateMaterialID(i)){
                        auto* m=materials.at(animation.getCRegUpdateMaterialID(i));const auto id=animation.mAnmCRegKeyTable[i].mColorId;
                        std::printf("Retail BRK %s:%s material=%s model=%.8s stages=%u block=%08X color=%u destination=%p\n",path,af.path.c_str(),names.getName(m->mIndex),model.data(),unsigned(m->mTevBlock->getTevStageNum()),unsigned(m->mTevBlock->getType()),unsigned(id),static_cast<void*>(m->mTevBlock->getTevColor(id)));
                        if(!m->mTevBlock->getTevColor(id))throw std::runtime_error("REPRO: native material lacks mutable BRK color destination");
                        assert(id<3);
                    }
                    for(unsigned i=0;i<animation.mKRegUpdateMaterialNum;++i)if(animation.isValidKRegUpdateMaterialID(i))assert(materials.at(animation.getKRegUpdateMaterialID(i))->mTevBlock->getTevKColor(animation.mAnmKRegKeyTable[i].mColorId));
                    assert(table.entryTevRegAnimator(&animation)==0);
                    for(float frame:{0.f,.5f,1.f,2.f,float(animation.mFrameMax)/2,float(animation.mFrameMax)}){
                        animation.mFrame=frame;for(auto* m:materials)m->mMaterialAnm->calc(m);
                        std::vector<std::unique_ptr<NativeMaterialCommands>> commands;for(auto* m:materials)commands.emplace_back(new NativeMaterialCommands(*m,heap));
                        for(unsigned i=0;i<animation.mCRegUpdateMaterialNum;++i)if(animation.isValidCRegUpdateMaterialID(i)){
                            unsigned material=animation.getCRegUpdateMaterialID(i),id=animation.mAnmCRegKeyTable[i].mColorId;GXColorS10 expected{};animation.getTevColorReg(i,&expected);
                            auto* actual=materials[material]->mTevBlock->getTevColor(id);assert(!std::memcmp(actual,&expected,sizeof(expected)));
                            auto emitted=commands[material]->state().tevColor(id+1);assert((emitted==std::array<int,4>{expected.r,expected.g,expected.b,expected.a}));++updates;
                        }
                        for(unsigned i=0;i<animation.mKRegUpdateMaterialNum;++i)if(animation.isValidKRegUpdateMaterialID(i)){
                            unsigned material=animation.getKRegUpdateMaterialID(i),id=animation.mAnmKRegKeyTable[i].mColorId;GXColor expected{};animation.getTevKonstReg(i,&expected);
                            auto* actual=materials[material]->mTevBlock->getTevKColor(id);assert(!std::memcmp(actual,&expected,sizeof(expected)));
                            auto emitted=commands[material]->state().tevColor(id,true);assert((emitted==std::array<int,4>{expected.r,expected.g,expected.b,expected.a}));++updates;
                        }
                    }
                    assert(table.removeTevRegAnimator(&animation)==0);++resources;
                }
                for(auto* m:materials){m->mMaterialAnm=nullptr;for(unsigned slot=0;slot<8;++slot)delete m->mTexGenBlock->getTexMtx(slot);delete m->mColorBlock;delete m->mTexGenBlock;delete m->mTevBlock;delete m->mIndBlock;delete m->mPEBlock;delete m;}
            }
            Native::gpuResources().remove(textures->storage.data());j3dSys.setTexture(previous);
        }
    }
    assert(resources>=2 && updates>0 && oneStage>0 && heap.check());
    std::printf("PASS original retail BRK attachment/material calc/removal, authored stage counts and emitted GX colors: %zu animations, %zu register checks, %zu one-stage materials\n",resources,updates,oneStage);
}
