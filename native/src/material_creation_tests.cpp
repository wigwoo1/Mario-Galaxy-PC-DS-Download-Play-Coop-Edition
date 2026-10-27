#include "native_material_init.hpp"
#include "native_model_blocks.hpp"
#include "native_texture_tables.hpp"
#include "native_material_commands.hpp"
#include "gx_command_texture.hpp"
#include "preview.hpp"
#include <set>
#include <Native/GpuResources.hpp>
#include <Native/GxCommandState.hpp>
#include <Native/TevStage.hpp>
#include <JSystem/J3DGraphBase/J3DMaterial.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <cassert>
#include <cstdio>
#define NOMINMAX
#include <windows.h>
#include "brk_material_tests.hpp"

int runMaterialChecks(int argc, char** argv) {
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    assert(argc == 2);
    std::vector<uint8_t> emptyTextureTable(64);
    std::memcpy(emptyTextureTable.data(),"TEX1",4);
    emptyTextureTable[7]=64; // Wii TEX1 block size, with zero texture headers.
    NativeTextureTables emptyTextures(emptyTextureTable.data(),emptyTextureTable.size());
    assert(emptyTextures.count==0 && emptyTextures.headersOffset==0 && emptyTextures.storage.size()==64);
    alignas(32) static unsigned char storage[8*1024*1024]{};
    JKRExpHeap heap(storage, sizeof(storage), nullptr, false);
    JKRHeap::sRootHeap = &heap;
    JKRHeap::sSystemHeap = &heap;
    heap.becomeCurrentHeap();
    WiiAssets disc(std::filesystem::u8path(argv[1]));
    verifyRetailBrkMaterials(disc,heap);
    Rarc archive(disc.readFile("/ObjectData/Mario.arc"));
    unsigned created = 0;
    for (const auto& file : archive.files) {
        auto bytes = archive.readFile(file.path);
        if (bytes.size() < 32 || std::memcmp(bytes.data(), "J3D2", 4)) continue;
        NativeModelBlocks modelBlocks(bytes.data(),bytes.size());
        auto rejectModel=[&](std::vector<uint8_t> bad){bool rejected=false;try{NativeModelBlocks invalid(bad.data(),bad.size());}catch(const std::runtime_error&){rejected=true;}assert(rejected);};
        auto badModel=bytes;std::memset(badModel.data()+8,0xff,4);rejectModel(badModel);
        badModel=bytes;std::memset(badModel.data()+12,0xff,4);rejectModel(badModel);
        assert(!modelBlocks.blocks.empty());
        badModel=bytes;std::memset(badModel.data()+modelBlocks.blocks[0].offset+4,0,4);rejectModel(badModel);
        badModel=bytes;std::memset(badModel.data()+modelBlocks.blocks[0].offset+4,0xff,4);rejectModel(badModel);
        std::unique_ptr<NativeTextureTables> textures;
        for(const auto& block:modelBlocks.blocks){
            auto scan=block.offset,length=block.length;
            if(!std::memcmp(bytes.data()+scan,"TEX1",4)){
                textures=std::make_unique<NativeTextureTables>(bytes.data()+scan,length);
                std::vector<uint8_t> bad(bytes.begin()+scan,bytes.begin()+scan+length);
                std::memset(bad.data()+textures->headersOffset+28,0xff,4);
                bool rejected=false;try{NativeTextureTables invalid(bad.data(),bad.size());}catch(const std::runtime_error&){rejected=true;}assert(rejected);
            }
        }
        assert(textures && textures->count);
        J3DTexture originalTextures(textures->count,textures->headers());
        auto* previousTexture=j3dSys.getTexture();j3dSys.setTexture(&originalTextures);
        Native::gpuResources().add(textures->storage.data(),textures->storage.size());
        std::set<unsigned> renderedTextures;
        for (const auto& block:modelBlocks.blocks) {
            auto pos=block.offset,length=block.length;
            if (!std::memcmp(bytes.data()+pos, "MAT3", 4)) {
                NativeMaterialInitTables tables(bytes.data()+pos, length);
                J3DMaterialFactory factory(tables);
                auto freeBefore=heap.getTotalFreeSize();
                for (unsigned index = 0; index < tables.header.materialCount; ++index) {
                    auto* material = factory.createNormalMaterial(nullptr, index, 0);
                    assert(material && material->mColorBlock && material->mTexGenBlock
                           && material->mTevBlock && material->mIndBlock && material->mPEBlock);
                    assert(material->mIndex == index);
                    assert(material->mMaterialMode == factory.getMaterialMode(index));
                    assert(material->mTexGenBlock->getTexGenNum() == factory.newTexGenNum(index));
                    assert(material->mColorBlock->getColorChanNum() == factory.newColorChanNum(index));
                    assert(material->mColorBlock->getCullMode() == factory.newCullMode(index));
                    assert(material->mTevBlock->getTevStageNum() == factory.newTevStageNum(index));
                    unsigned slots=std::min(8u,factory.countStages(index));
                    for(unsigned slot=0;slot<slots;++slot){
                        assert(material->mTevBlock->getTexNo(slot)==factory.newTexNo(index,slot));
                        assert(factory.newTexNo(index,slot)==65535 || factory.newTexNo(index,slot)<textures->count);
                    }
                    Mtx identity={{1,0,0,0},{0,1,0,0},{0,0,1,0}};
                    // Material::calc transfers current matrix indices to its
                    // associated shape even before geometry is loaded.
                    J3DShape shape;
                    material->mShape=&shape;
                    material->calc(identity);
                    NativeMaterialCommands commands(*material,heap);
                    assert(commands.bytes().size()%32==0);
                    const auto& state=commands.state();
                    assert(((state.bp(0)>>10)&15)+1==factory.newTevStageNum(index));
                    if(state.tevStageCount()>1)for(unsigned stage=0;stage<state.tevStageCount();++stage){
                        auto selection=state.tevKonst(stage);
                        assert(selection.color==material->mTevBlock->getTevKColorSel(stage));
                        assert(selection.alpha==material->mTevBlock->getTevKAlphaSel(stage));
                    }
                    Native::TevFragmentInputs fragment;
                    for(unsigned stage=0;stage<state.tevStageCount();++stage){
                        // Original stage objects store BP commands in byte order,
                        // regardless of the host CPU's integer endianness.
                        const auto* packed=reinterpret_cast<const u8*>(material->mTevBlock->getTevStage(stage));
                        for(unsigned command=0;command<2;++command){
                            const auto* bytes=packed+command*4;
                            assert(bytes[0]==0xc0+stage*2+command);
                            assert(state.bp(bytes[0])==(u32(bytes[1])<<16|u32(bytes[2])<<8|bytes[3]));
                        }
                        auto order=state.tevOrder(stage);
                        fragment.raster[stage]=order.channel==7?Native::TevRgba{}:Native::TevRgba{255,255,255,255};
                        if(order.enabled){auto texture=decodeCommandTexture(state,order.texture,Native::gpuResources());auto pixel=texture.pixels.at(size_t(texture.height/2)*texture.width+texture.width/2);fragment.textures[stage]={pixel.r,pixel.g,pixel.b,pixel.a};}
                    }
                    auto shaded=Native::evaluateTevProgram(state,fragment);
                    std::printf("INFO original material %u sampled TEV fragment: %d %d %d %d\n",index,shaded[0],shaded[1],shaded[2],shaded[3]);
                    for(unsigned slot=0;slot<slots;++slot){
                        auto number=factory.newTexNo(index,slot);if(number==65535)continue;
                        assert(number<textures->count);auto& header=textures->headers()[number];
                        auto bound=state.texture(slot);assert(bound.width==header.mWidth && bound.height==header.mHeight && bound.format==header.mFormat);
                        auto length=gxEncodedSize(header.mWidth,header.mHeight,header.mFormat);
                        auto* source=reinterpret_cast<uint8_t*>(&header)+header.mImageDataOffset;
                        assert(!std::memcmp(Native::gpuResources().resolve(bound.address,length),source,length));
                        if(renderedTextures.insert(number).second){
                            auto decoded=decodeCommandTexture(state,slot,Native::gpuResources());
                            // Decode a second copy from the original disk header,
                            // independent of native-endian table binding.
                            size_t textureBase=0;
                            // Locate TEX1 again in the untouched retail model.
                            for(size_t at=32;at<bytes.size();){auto size=be32(bytes.data()+at+4);if(!std::memcmp(bytes.data()+at,"TEX1",4)){textureBase=at;break;}at+=size;}
                            assert(textureBase);
                            std::vector<uint8_t> diskTexture(bytes.begin()+textureBase,bytes.begin()+textureBase+be32(bytes.data()+textureBase+4));
                            auto reference=decodeBti(BinaryView(diskTexture),textures->headersOffset+number*32);
                            assert(decoded.width==reference.width && decoded.height==reference.height && decoded.pixels.size()==reference.pixels.size());
                            assert(!std::memcmp(decoded.pixels.data(),reference.pixels.data(),decoded.pixels.size()*sizeof(Pixel)));
                            verifyRendering(decoded,true);verifyRendering(decoded,false);
                            std::printf("PASS original material texture %u GPU readback\n",number);
                        }
                    }
                    std::printf("PASS original material %u commands: %zu bytes\n",index,commands.bytes().size());
                    // Original SDK objects use bulk arena teardown. This fixture
                    // instead releases each owned allocation to check heap balance.
                    for(unsigned slot=0;slot<8;++slot)delete material->mTexGenBlock->getTexMtx(slot);
                    delete material->mColorBlock;
                    delete material->mTexGenBlock;
                    delete material->mTevBlock;
                    delete material->mIndBlock;
                    delete material->mPEBlock;
                    ++created;
                    delete material;
                    assert(heap.check() && heap.getTotalFreeSize()==freeBefore);
                }
            }
        }
        Native::gpuResources().remove(textures->storage.data());j3dSys.setTexture(previousTexture);
        assert(!renderedTextures.empty());
    }
    assert(created == 9 && heap.check());
    std::printf("PASS original material creation, commands and bound texture GPU readback: %u Mario materials (no model geometry drawing)\n", created);
    return 0;
}
int main(int argc,char** argv){try{return runMaterialChecks(argc,argv);}catch(const std::exception& error){std::fprintf(stderr,"Material check failed: %s\n",error.what());return 1;}}


