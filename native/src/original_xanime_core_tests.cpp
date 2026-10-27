#include "native_owned_bti.hpp"
#include "native_texture_srt_animation.hpp"
#include <Native/HeapGpuRange.hpp>
#include <Native/HeapJMapOwner.hpp>
#include "native_model_projection.hpp"
#include "native_model_texgen.hpp"
#include "native_model_texture.hpp"
#include "gx_command_texture.hpp"
#include <windows.h>
#include <cstdlib>
#include <Game/Util/MtxUtil.hpp>
#include <Game/Util/MathUtil.hpp>
namespace MR {void copyMemory(void*,const void*,unsigned int);}
#include <revolution/gd/GDTransform.h>
#include <revolution/gd/GDLight.h>
#include <Game/Player/J3DModelX.hpp>
#include <Native/GDCurrentScope.hpp>
#include <JSystem/J3DGraphBase/J3DDrawBuffer.hpp>
#include "animation.hpp"
#include "native_joint_tables.hpp"
#include "native_joint_resource.hpp"
#include "native_material_init.hpp"
#include "native_texture_tables.hpp"
#include "native_material_commands.hpp"
#include "native_shape_tables.hpp"
#include "native_shape_resource.hpp"
#include "native_vertex_resource.hpp"
#include "native_shape_vertices.hpp"
#include "native_shape_stream.hpp"
#include "mesh.hpp"
#include "native_draw_matrix_resource.hpp"
#include "native_envelope_resource.hpp"
#include <Native/GpuResources.hpp>
#include <Native/GxCommandState.hpp>
#include <revolution/gd/GDBase.h>
#include <JSystem/J3DGraphBase/J3DShapeDraw.hpp>
#include "native_model_blocks.hpp"
#include "native_model_resource.hpp"
#include "native_matrix_buffer_resource.hpp"
#include "native_model_packets.hpp"
#include <Game/Util/ModelUtil.hpp>
#include "native_model_instance.hpp"
#include "native_owned_model_data.hpp"
#include "native_texture_pattern_animation.hpp"
#include <JSystem/J3DGraphAnimator/J3DMaterialAnm.hpp>
#include "native_jkr_archive_port.hpp"
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <Native/BckAnimation.hpp>
#include <Game/Animation/XanimeCore.hpp>
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <JSystem/J3DGraphAnimator/J3DMtxBuffer.hpp>
#include <JSystem/J3DGraphAnimator/J3DJointTree.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <cassert>
#include <cstdio>
namespace MR {bool isUseFur(const J3DModelData*);void invalidateMtxCalc(J3DModelData*);void invalidateJointCallback(J3DModelData*);}
#include <dbghelp.h>
static LONG WINAPI modelCrashDiagnostic(EXCEPTION_POINTERS* exception) {
    if(exception->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    HANDLE process=GetCurrentProcess();
    SymInitialize(process,"D:\\Desktop\\Games\\Mario galaxy\\native\\build",TRUE);
    char executable[MAX_PATH];GetModuleFileNameA(NULL,executable,MAX_PATH);
    SymLoadModuleEx(process,NULL,executable,NULL,reinterpret_cast<DWORD64>(GetModuleHandleA(NULL)),0,NULL,0);
    char storage[sizeof(SYMBOL_INFO)+1024]{};
    auto* symbol=reinterpret_cast<SYMBOL_INFO*>(storage);
    symbol->SizeOfStruct=sizeof(SYMBOL_INFO);symbol->MaxNameLen=1024;
    DWORD64 displacement=0,address=exception->ContextRecord->Rip;
    std::fprintf(stderr,"CRASH RIP %llx module offset %llx access %llu target %llx\n",
        address,address-reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL)),
        exception->ExceptionRecord->ExceptionInformation[0],exception->ExceptionRecord->ExceptionInformation[1]);
    if(SymFromAddr(process,address,&displacement,symbol))
        std::fprintf(stderr,"CRASH symbol %s + %llu\n",symbol->Name,displacement);
    return EXCEPTION_CONTINUE_SEARCH;
}
#include "extra_model_matrices_tests.hpp"
int main(int argc,char** argv){ AddVectoredExceptionHandler(1,modelCrashDiagnostic); SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX); _set_error_mode(_OUT_TO_STDERR); _set_abort_behavior(0,_WRITE_ABORT_MSG|_CALL_REPORTFAULT);
    std::setvbuf(stdout,nullptr,_IONBF,0);assert(argc==2);alignas(32) static unsigned char storage[8*1024*1024]{};
    JKRExpHeap heap(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&heap;JKRHeap::sSystemHeap=&heap;heap.becomeCurrentHeap();
    {u8 outerBytes[128]{},privateBytes[128]{};GDLObj outer;
      GDInitGDLObj(&outer,outerBytes,sizeof(outerBytes));GDSetCurrent(&outer);GDWriteBPCmd(0x41000011);
      J3DDisplayListObj list;list.setSingleDisplayList(privateBytes,sizeof(privateBytes));list.beginDL();GDWriteBPCmd(0x41000022);list.endDL();
      assert(__GDCurrentDL==&outer);GDWriteBPCmd(0x41000033);assert(GDGetCurrOffset()==10&&outerBytes[4]==0x11&&outerBytes[9]==0x33);
      assert(privateBytes[0]==0x61&&privateBytes[4]==0x22);
      list.beginPatch();GDWriteBPCmd(0x41000044);list.endPatch();assert(__GDCurrentDL==&outer&&GDGetCurrOffset()==10);GDSetCurrent(nullptr);
      list.beginDL();GDWriteBPCmd(0x41000055);list.endDL();assert(__GDCurrentDL==nullptr);
      std::puts("PASS original J3D list and patch writer restoration: outer commands preserved, private bytes isolated, initially empty writer restored");}
    {auto* listHeap=JKRExpHeap::create(64*1024,&heap,false);assert(listHeap);listHeap->becomeCurrentHeap();
      u32 firstAddress=0,secondAddress=0;
      {J3DDisplayListObj list;assert(list.newDisplayList(64)==0);
        firstAddress=Native::gpuResources().byteAddress(list.mpDisplayList[0]);secondAddress=Native::gpuResources().byteAddress(list.mpDisplayList[1]);
        assert(firstAddress!=secondAddress);list.beginDL();GDWriteBPCmd(0x41000022);list.endDL();
        auto written=Native::gpuResources().byteAddress(list.mpDisplayList[0]);assert(Native::gpuResources().resolve(written,32)[4]==0x22);
        list.beginPatch();GDWriteBPCmd(0x41000033);list.endPatch();
        written=Native::gpuResources().byteAddress(list.mpDisplayList[0]);assert(Native::gpuResources().resolve(written,32)[4]==0x33);
      }
      {J3DDisplayListObj list;assert(list.newSingleDisplayList(64)==0);list.beginDL();GDWriteBPCmd(0x41000044);list.endDL();
        assert(list.single_To_Double()==0);auto second=Native::gpuResources().byteAddress(list.mpDisplayList[1]);assert(Native::gpuResources().resolve(second,64)[4]==0x44);}
      heap.becomeCurrentHeap();JKRHeap::destroy(listHeap);
      for(auto address:{firstAddress,secondAddress}){bool removed=false;try{Native::gpuResources().resolve(address,1);}catch(const std::out_of_range&){removed=true;}assert(removed);}
      std::puts("PASS original J3D allocated lists: double/single registration, edited snapshots, conversion and heap-teardown removal");}
    {auto* matrixHeap=JKRExpHeap::create(64*1024,&heap,false);assert(matrixHeap);matrixHeap->becomeCurrentHeap();
      J3DModelData fixture;fixture.mJointTree.mDrawMtxData.mEntryNum=2;J3DMtxBuffer matrices;
      assert(matrices.createDoubleDrawMtx(&fixture,2)==0);std::vector<u32> addresses;
      for(unsigned side=0;side<2;++side)for(unsigned view=0;view<2;++view){
        auto* draw=matrices.mpDrawMtxArr[side][view];auto* normal=matrices.mpNrmMtxArr[side][view];
        auto drawAddress=Native::gpuResources().byteAddress(draw);auto normalAddress=Native::gpuResources().byteAddress(normal);addresses.push_back(drawAddress);addresses.push_back(normalAddress);
        draw[0][0][0]=1.5f;normal[0][0][0]=2.5f;DCStoreRangeNoSync(draw,2*sizeof(Mtx));DCStoreRange(normal,2*sizeof(Mtx33));
        const u8 expectedDraw[]={0x3f,0xc0,0,0},expectedNormal[]={0x40,0x20,0,0};
        assert(!std::memcmp(Native::gpuResources().resolve(drawAddress,4),expectedDraw,4));assert(!std::memcmp(Native::gpuResources().resolve(normalAddress,4),expectedNormal,4));
        draw[0][0][0]=-0.5f;DCStoreRange(draw,2*sizeof(Mtx));const u8 updated[]={0xbf,0,0,0};assert(!std::memcmp(Native::gpuResources().resolve(drawAddress,4),updated,4));
      }
      auto* released=matrices.mpDrawMtxArr[0][0];assert(Native::releaseHeapGpuRange(released));assert(!Native::hasHeapGpuRange(released));assert(!Native::releaseHeapGpuRange(released));
      JKRHeap::free(released,matrixHeap);matrices.mpDrawMtxArr[0][0]=nullptr;
      for(unsigned side=0;side<2;++side){delete[] matrices.mpDrawMtxArr[side];delete[] matrices.mpNrmMtxArr[side];}
      heap.becomeCurrentHeap();JKRHeap::destroy(matrixHeap);
      for(auto address:addresses){bool removed=false;try{Native::gpuResources().resolve(address,1);}catch(const std::out_of_range&){removed=true;}assert(removed);}
      std::puts("PASS original matrix buffers: both sides/two views, native float byte order, cache-flush updates and heap teardown");}
    WiiAssets disc(std::filesystem::u8path(argv[1]));verifyRetailExtraModelMatrices(disc,heap);Rarc archive(disc.readFile("/ObjectData/MarioAnime.arc"));
    auto bytes=archive.readFile("/run.bck");auto decoded=decodeBck(BinaryView(bytes));
    std::unique_ptr<J3DAnmBase> loaded(Native::createBckAnimation(bytes.data(),bytes.size()));
    auto* animation=static_cast<J3DAnmTransformKey*>(loaded.get());
    XanimeCore core(1,u32(decoded.joints.size()),0);core.setBck(0,animation);
    std::vector<std::array<float,12>> storageMatrices(decoded.joints.size());
    J3DMtxBuffer buffer;buffer.mpAnmMtx=reinterpret_cast<Mtx*>(storageMatrices.data());
    auto* previousBuffer=J3DMtxCalc::getMtxBuffer();auto* previousJoint=J3DMtxCalc::getJoint();
    J3DMtxCalc::setMtxBuffer(&buffer);J3DJoint joint;J3DMtxCalc::setJoint(&joint);
    size_t samples=0;
    for(unsigned tick=0;tick<120;++tick){core.mFrameRatio=float(tick%30)/30;core.updateFrame();
        for(size_t index=0;index<decoded.joints.size();++index){joint.mJntNo=u16(index);TVec3f scale,translation;core.calcSingle(&scale,&translation);
            for(unsigned axis=0;axis<3;++axis){auto& tracks=decoded.joints[index];float expected=tracks[axis*3].sample(animation->mFrame,1);
                assert(std::fabs((&scale.x)[axis]-expected)<0.0001f*(1+std::fabs(expected)));
                expected=tracks[axis*3+2].sample(animation->mFrame,0);assert(std::fabs((&translation.x)[axis]-expected)<0.0001f*(1+std::fabs(expected)));
            }
            auto matrix=buffer.getAnmMtx(int(index));
            for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col){float dot=0;for(unsigned k=0;k<3;++k)dot+=matrix[row][k]*matrix[col][k];assert(std::fabs(dot-(row==col?1:0))<0.0001f);}
            ++samples;
        }
    }
    size_t transitions=0;
    core.mTrackList[0].setFrame(0.25f);core.mFrameRatio=0.75f;core.updateFrame();
    assert(animation->mFrame==decoded.duration*0.25f);
    core.mTrackList[0]._C=0;core.updateFrame();assert(animation->mFrame==decoded.duration*0.75f);
    for(size_t index=0;index<decoded.joints.size();++index){
        joint.mJntNo=u16(index);core._1C=1;core._29=0;animation->mFrame=2;
        TVec3f oldScale,oldTranslation;core.calcSingle(&oldScale,&oldTranslation);
        auto oldPose=core.mJointList[index]._28;
        J3DTransformInfo target;animation->mFrame=12;animation->getTransform(u16(index),&target);
        for(float ratio:{0.0f,0.25f,0.5f,1.0f}){
            core.mJointList[index]._28=oldPose;core._29=1;core._1C=ratio;
            TVec3f scale,translation;core.calcSingle(&scale,&translation);
            for(unsigned axis=0;axis<3;++axis){
                float expected=(&oldScale.x)[axis]*(1-ratio)+(&target.mScale.x)[axis]*ratio;
                assert(std::fabs((&scale.x)[axis]-expected)<0.0001f*(1+std::fabs(expected)));
                expected=(&oldTranslation.x)[axis]*(1-ratio)+(&target.mTranslate.x)[axis]*ratio;
                assert(std::fabs((&translation.x)[axis]-expected)<0.0001f*(1+std::fabs(expected)));
            }
            assert(core.mJointList[index]._50.x==target.mTranslate.x);
            auto cached=core.mJointList[index]._28;
            core.setBck(0,nullptr);TVec3f cachedScale,cachedTranslation;core.calcSingle(&cachedScale,&cachedTranslation);
            assert(cachedScale.x==cached._0.x && cachedScale.y==cached._0.y && cachedScale.z==cached._0.z);
            assert(cachedTranslation.x==cached._C.x && cachedTranslation.y==cached._C.y && cachedTranslation.z==cached._C.z);
            core.setBck(0,animation);++transitions;
        }
    }
    size_t worldMatrices=0;
    std::vector<u8> scaleFlags(decoded.joints.size());buffer.mpScaleFlagArr=scaleFlags.data();
    Mtx savedCurrent;std::memcpy(savedCurrent,J3DSys::mCurrentMtx,sizeof(Mtx));
    Vec savedParent=J3DSys::mParentS;auto* savedCalc=j3dSys.mCurrentMtxCalc;
    core._4=2;core._1C=1;core._29=0;
    for(unsigned tick=0;tick<30;++tick){
        animation->mFrame=float(tick);
        for(size_t index=0;index<decoded.joints.size();++index){
            joint.mJntNo=u16(index);TVec3f scale,translation;core.calcSingle(&scale,&translation);
            Mtx local;std::memcpy(local,buffer.getAnmMtx(int(index)),sizeof(Mtx));
            for(unsigned row=0;row<3;++row){for(unsigned col=0;col<3;++col)local[row][col]*=(&scale.x)[col];local[row][3]=(&translation.x)[row];}
            const float parent[3][4]={{0,-1,0,10},{1,0,0,-7},{0,0,1,3}};
            for(unsigned compensate=0;compensate<2;++compensate){
                joint.mScaleCompensate=u8(compensate);J3DSys::mParentS={2,3,4};
                std::memcpy(J3DSys::mCurrentMtx,parent,sizeof(Mtx));core.calc();
                auto actual=buffer.getAnmMtx(int(index));
                for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col){
                    float expected=col==3?parent[row][3]:0;
                    for(unsigned k=0;k<3;++k)expected+=parent[row][k]*local[k][col]/((compensate && col<3)?float(k+2):1.0f);
                    assert(std::fabs(actual[row][col]-expected)<0.0001f*(1+std::fabs(expected)));
                }
                assert(J3DSys::mParentS.x==scale.x && J3DSys::mParentS.y==scale.y && J3DSys::mParentS.z==scale.z);
                assert(scaleFlags[index]==u8(scale.x==1 && scale.y==1 && scale.z==1));++worldMatrices;
            }
        }
    }
    {const char* archives[]={
#include "../build/btk-stationed-archives.inc"
     };size_t checked=0;for(const char* name:archives){
      const std::string path=name;if(std::none_of(disc.files.begin(),disc.files.end(),[&](const AssetFile& file){return file.path==path;}))continue;Rarc source(disc.readFile(path));
      for(const auto& file:source.files)if(file.path.size()>4&&file.path.substr(file.path.size()-4)==".btk"){
       auto data=source.readFile(file.path);try{NativeTextureSrtAnimation animation(data.data(),data.size());++checked;
        if(be32(data.data()+36)==data.size()-32+4){
         auto bad=data;const u32 pastEnd=u32(data.size()-32-8);for(int byte=0;byte<4;++byte)bad[80+byte]=u8(pastEnd>>(24-byte*8));
         bool rejected=false;try{NativeTextureSrtAnimation truncated(bad.data(),bad.size());}catch(const std::exception&){rejected=true;}assert(rejected);
        }
       }
       catch(const std::exception& error){std::fprintf(stderr,"BTK decode rejected %s:%s bytes=%zu: %s\n",name,file.path.c_str(),data.size(),error.what());for(size_t i=32;i<128&&i<data.size();++i)std::fprintf(stderr,"%02X",data[i]);std::fprintf(stderr,"\n");return 31;}
      }
     }std::printf("PASS stationed-object BTK decoding: %zu animations\n",checked);}
    {Rarc star(disc.readFile("/ObjectData/StarPiece.arc"));std::vector<u8> gift,starModel;
      for(const auto& file:star.files){if(file.path.find("gift.btk")!=std::string::npos)gift=star.readFile(file.path);if(file.path.find(".bdl")!=std::string::npos)starModel=star.readFile(file.path);}
      assert(!gift.empty()&&!starModel.empty());const auto originalGift=gift;
      NativeTextureSrtAnimation btk(gift.data(),gift.size());BinaryView raw(gift);auto block=raw.sub(32,raw.u32(36));assert(block.tag(0)=="TTK1");
      auto model=createNativeModelData(starModel.data(),starModel.size(),heap);btk.searchUpdateMaterialID(model.get());assert(btk.getUpdateMaterialNum()>0);
      size_t compared=0;
      for(unsigned material=0;material<btk.getUpdateMaterialNum();++material){assert(btk.isValidUpdateMaterialID(material));
       std::array<AnimationTrack,5> reference;
       const unsigned axes[]={0,1,2,0,1},kinds[]={0,0,1,2,2};
       for(unsigned channel=0;channel<5;++channel){unsigned kind=kinds[channel];size_t at=block.u32(20)+(material*3+axes[channel])*18+kind*6;
        unsigned count=block.u16(at),offset=block.u16(at+2),type=block.u16(at+4),stride=type?4:3,width=kind==1?2:4;size_t base=block.u32(40+kind*4);
        auto value=[&](unsigned index){return kind==1?float(s16(block.u16(base+index*width))):block.f32(base+index*width);};
        for(unsigned key=0;key<count;++key){unsigned i=offset+key*stride;reference[channel].keys.push_back(count==1?AnimationKey{0,value(offset),0,0}:AnimationKey{value(i),value(i+1),value(i+2),value(i+(type?3:2))});}
       }
       for(float frame:{0.f,.5f,5.f,float(btk.mFrameMax)}){J3DTextureSRTInfo actual;btk.calcTransform(frame,u16(material),&actual);
        const float expected[]={reference[0].sample(frame,1),reference[1].sample(frame,1),reference[3].sample(frame,0),reference[4].sample(frame,0)};
        const float got[]={actual.mScaleX,actual.mScaleY,actual.mTranslationX,actual.mTranslationY};
        for(unsigned i=0;i<4;++i){assert(std::fabs(got[i]-expected[i])<.0001f*(1+std::fabs(expected[i])));++compared;}
        const auto rotation=s16(u16(uint32_t(int32_t(reference[2].sample(frame,0)))*(1u<<btk.mDecShift)));assert(actual.mRotation==rotation);
       }
      }
      assert(gift==originalGift);auto reject=[&](std::vector<u8> bad){bool rejected=false;try{NativeTextureSrtAnimation invalid(bad.data(),bad.size());}catch(const std::exception&){rejected=true;}assert(rejected);};
      reject(std::vector<u8>(gift.begin(),gift.begin()+31));auto bad=gift;bad[0]='X';reject(bad);bad=gift;bad[44]=0;bad[45]=1;reject(bad);
      auto write=[&](std::vector<u8>& bytes,size_t at,u32 value){for(int i=3;i>=0;--i){bytes[at+i]=u8(value);value>>=8;}};
      bad=gift;write(bad,52,0xFFFFFFF0);reject(bad);bad=gift;size_t first=32+block.u32(20);bad[first+2]=bad[first+3]=255;reject(bad);
      auto post=gift;std::copy(post.begin()+44,post.begin()+52,post.begin()+84);std::copy(post.begin()+52,post.begin()+84,post.begin()+92);
      NativeTextureSrtAnimation postBtk(post.data(),post.size());assert(postBtk.getPostUpdateMaterialNum()==btk.getUpdateMaterialNum());
      gift.clear();gift.shrink_to_fit();J3DTextureSRTInfo retained;btk.calcTransform(5,0,&retained);assert(std::isfinite(retained.mTranslationX));
      std::printf("PASS StarPiece Gift BTK: native original evaluation vs independent curves (%zu components), material remapping, post tracks, source release and malformed data\n",compared);
    }
    Rarc modelArchive(disc.readFile("/ObjectData/Mario.arc"));auto modelBytes=modelArchive.readFile("/mario.bdl");
    auto checkMaterialGroups=[](const std::vector<uint8_t>& input,const char* label){
        NativeModelBlocks sections(input.data(),input.size());std::vector<u16> authored;
        for(auto block:sections.blocks){BinaryView table(input,block.offset,block.length);if(table.tag(0)!="MAT3")continue;
            for(unsigned i=0;i<table.u16(8);++i)authored.push_back(table.u16(table.u32(16)+2*i));}
        NativeModelResource retained(input);J3DModelData view;retained.bind(view);
        assert(view.getMaterialNum()==authored.size());std::vector<u16> unique;
        for(auto id:authored)if(std::find(unique.begin(),unique.end(),id)==unique.end())unique.push_back(id);
        assert(view.mMaterialTable.mUniqueMatNum==unique.size());
        for(unsigned i=0;i<authored.size();++i)for(unsigned j=0;j<authored.size();++j)
            assert((view.getMaterialNodePointer(i)->getDiffFlag()==view.getMaterialNodePointer(j)->getDiffFlag())==(authored[i]==authored[j]));
        std::printf("PASS original material grouping %s: %zu entries / %zu authored groups\n",label,authored.size(),unique.size());
    };
    checkMaterialGroups(modelBytes,"Mario");
    {Rarc star(disc.readFile("/ObjectData/StarPiece.arc"));for(const auto& file:star.files)if(file.path.find(".bdl")!=std::string::npos)checkMaterialGroups(star.readFile(file.path),"StarPiece");}
    {auto duplicate=modelBytes;NativeModelBlocks sections(duplicate.data(),duplicate.size());for(auto block:sections.blocks){BinaryView table(duplicate,block.offset,block.length);if(table.tag(0)!="MAT3")continue;
        assert(table.u16(8)>1);auto remap=block.offset+table.u32(16);duplicate[remap+2]=duplicate[remap];duplicate[remap+3]=duplicate[remap+1];}
        checkMaterialGroups(duplicate,"shared-material variant");}
    auto rig=decodeRig(BinaryView(modelBytes));assert(rig.joints.size()==decoded.joints.size());
    std::unique_ptr<NativeJointResource> retainedJoints;
    NativeModelBlocks modelBlocks(modelBytes.data(),modelBytes.size());std::unique_ptr<NativeJointTables> jointTables;
    for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="JNT1"){jointTables=std::make_unique<NativeJointTables>(section);retainedJoints=std::make_unique<NativeJointResource>(section);}}
    assert(jointTables && jointTables->names);JUTNameTab jointNames(jointTables->names->resource());
    for(u16 i=0;i<jointNames.mNameNum;++i){const char* name=jointNames.getName(i);assert(name && jointNames.getIndex(name)==i);}assert(jointNames.getIndex("__missing_native_joint__")==-1);
    {const uint8_t missingOffset[]={0,1,0,0,0,0,255,255};std::vector<uint8_t> malformed(missingOffset,missingOffset+8);bool rejected=false;
        try{NativeNameTable bad{BinaryView(malformed)};}catch(const std::exception&){rejected=true;}assert(rejected);
        malformed={0,1,0,0,0,0,0,8,'x'};rejected=false;try{NativeNameTable bad{BinaryView(malformed)};}catch(const std::exception&){rejected=true;}assert(rejected);}
    std::printf("PASS original JUTNameTab: %u retail joint names, missing lookup and malformed bounds\n",jointNames.mNameNum);
    assert(jointTables);const auto* nativeJointBlock=jointTables->loaderBlock();J3DJointFactory jointFactory(*nativeJointBlock);
    JUTNameTab loaderNames(reinterpret_cast<const ResNTAB*>(reinterpret_cast<const uint8_t*>(nativeJointBlock)+reinterpret_cast<uintptr_t>(nativeJointBlock->mpNameTable)));
    for(u16 i=0;i<jointNames.mNameNum;++i)assert(!std::strcmp(loaderNames.getName(i),jointNames.getName(i)));
    jointTables->records.clear();jointTables->records.shrink_to_fit();jointTables->indices.clear();jointTables->indices.shrink_to_fit();jointTables->names.reset();
    assert(loaderNames.getIndex(loaderNames.getName(0))==0);
    for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="INF1")retainedJoints->buildHierarchy(section);}
    assert(retainedJoints);J3DJointTree retainedTree;retainedJoints->bind(retainedTree);auto& ownedJoints=retainedJoints->nodes;
    assert(retainedTree.getJointNum()==rig.joints.size());
    for(size_t i=0;i<rig.joints.size();++i){assert(retainedTree.getJointNodePointer(u16(i))==ownedJoints[i].get());auto& info=ownedJoints[i]->mTransformInfo;
        for(unsigned k=0;k<3;++k){assert((&info.mScale.x)[k]==rig.joints[i].scale[k]);assert((&info.mTranslate.x)[k]==rig.joints[i].translation[k]);assert((&info.mRotation.x)[k]==rig.joints[i].rotation[k]);}
        assert(retainedTree.getJointName()->getIndex(loaderNames.getName(u16(i)))==int(i));}
    std::unique_ptr<NativeTextureTables> retainedTextures;
    for(auto block:modelBlocks.blocks)if(BinaryView(modelBytes,block.offset,block.length).tag(0)=="TEX1")retainedTextures=std::make_unique<NativeTextureTables>(modelBytes.data()+block.offset,block.length);
    assert(retainedTextures);J3DTexture originalTextures(retainedTextures->count,retainedTextures->headers());
    auto* previousTexture=j3dSys.getTexture();j3dSys.setTexture(&originalTextures);
    Native::gpuResources().add(retainedTextures->storage.data(),retainedTextures->storage.size());
    std::vector<std::unique_ptr<J3DMaterial>> meshMaterials;std::vector<J3DMaterial*> materialPointers;
    std::unique_ptr<NativeShapeResource> retainedShapes;std::vector<J3DShape*> shapePointers;
    for(auto block:modelBlocks.blocks){if(BinaryView(modelBytes,block.offset,block.length).tag(0)=="MAT3"){
        NativeMaterialInitTables materials(modelBytes.data()+block.offset,block.length);J3DMaterialFactory factory(materials);
        for(unsigned i=0;i<materials.header.materialCount;++i){meshMaterials.emplace_back(factory.createNormalMaterial(nullptr,int(i),0));auto* material=meshMaterials.back().get();assert(material && material->mIndex==i && material->mTevBlock && material->mColorBlock);materialPointers.push_back(material);}
    }}
    for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="SHP1"){
        NativeShapeTables shapes(section);shapes.decodeGeometryTables(section);assert(shapes.indices.size()==9);J3DShapeBlock empty{};J3DShapeFactory factory(empty);shapes.bindGeometry(factory);factory.allocVcdVatCmdBuffer(u32(shapes.indices.size()));
        for(unsigned i=0;i<shapes.indices.size();++i){size_t p=section.u32(12)+section.u16(section.u32(16)+i*2)*40;
            assert(factory.getMtxGroupNum(i)==section.u16(p+2));assert(factory.getRadius(i)==section.f32(p+12));
            for(unsigned k=0;k<3;++k){assert((&factory.getMin(i).x)[k]==section.f32(p+16+k*4));assert((&factory.getMax(i).x)[k]==section.f32(p+28+k*4));}}
        for(unsigned i=0;i<shapes.indices.size();++i){std::unique_ptr<J3DShape> shape(factory.create(int(i),0,nullptr));assert(shape->mIndex==i && shape->mMtxGroupNum==factory.getMtxGroupNum(i));
            for(unsigned group=0;group<shape->mMtxGroupNum;++group){auto& record=shapes.drawRecords[shapes.records[shapes.indices[i]].mDrawInitDataIndex+group];assert(shape->mShapeMtx[group]);assert(shape->mShapeDraw[group]->getDisplayListSize()==record.mDisplayListSize);assert(!std::memcmp(shape->mShapeDraw[group]->getDisplayList(),shapes.displayLists.data()+record.mDisplayListIndex,record.mDisplayListSize));delete shape->mShapeMtx[group];delete shape->mShapeDraw[group];}
            delete[] shape->mShapeMtx;delete[] shape->mShapeDraw;shape->mShapeMtx=nullptr;shape->mShapeDraw=nullptr;}
        JKRHeap::free(factory.mVcdVatCmdBuffer,&heap);factory.mVcdVatCmdBuffer=nullptr;
        std::printf("PASS original shape factory: 9 retail records, %zu matrix groups, %zu draw records, %zu geometry bytes\n",shapes.matrixRecords.size(),shapes.drawRecords.size(),shapes.displayLists.size());
    }}
    for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="SHP1")retainedShapes=std::make_unique<NativeShapeResource>(section);}
    assert(materialPointers.size()==9 && retainedShapes);retainedShapes->registerGeometry();shapePointers=retainedShapes->pointers;assert(shapePointers.size()==9);
    std::unique_ptr<NativeVertexResource> retainedVertices;for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="VTX1")retainedVertices=std::make_unique<NativeVertexResource>(section);}assert(retainedVertices);
    auto* previousPositions=j3dSys.getVtxPos();auto* previousNormals=j3dSys.getVtxNrm();auto* previousColors=j3dSys.getVtxCol();
    retainedVertices->registerGpu();retainedVertices->bindCurrentArrays();
    retainedShapes->prepareVertexCommands(retainedVertices->data);
    retainedShapes->prepareVertexCommands(retainedVertices->data); // Refresh existing registration without duplicating GPU ranges.
    for(auto* shape:shapePointers)assert(static_cast<const u8*>(shape->getVcdVatCmd())[0]!=0);

    auto referenceModel=decodeModel(BinaryView(modelBytes));auto referenceMesh=decodeMesh(BinaryView(modelBytes),referenceModel,rig);size_t vertexChecks=0;
    for(auto* shape:shapePointers){Native::GxCommandState state;state.consume(static_cast<const u8*>(shape->getVcdVatCmd()),J3DShape::kVcdVatDLSize,&Native::gpuResources());
        assert(state.cpConfigured());auto format=state.positionFormat(0);assert(format.components==3 && format.descriptor>=2);
        assert(state.cp(0xa0)==Native::gpuResources().byteAddress(retainedVertices->data.mVtxPosArray));
        for(size_t index=0;index<referenceModel.positions.size();++index){auto position=state.indexedPosition(0,unsigned(index),Native::gpuResources());for(unsigned k=0;k<3;++k)assert(std::fabs(position[k]-referenceModel.positions[index][k])<0.0001f);++vertexChecks;}}
    std::printf("PASS original shape VCD/VAT: %zu indexed retail position comparisons\n",vertexChecks);
    for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="INF1")retainedJoints->attachMeshes(section,materialPointers,shapePointers);}
    for(unsigned i=0;i<9;++i){assert(materialPointers[i]->getJoint());assert(std::find(materialPointers.begin(),materialPointers.end(),shapePointers[i]->getMaterial())!=materialPointers.end());}
    std::printf("PASS retail hierarchy: 9 original material/joint/shape attachments\n");
    J3DJoint* root=retainedTree.getRootNode();assert(root);
    {std::vector<uint8_t> invalid(28);invalid[0]='I';invalid[1]='N';invalid[2]='F';invalid[3]='1';invalid[7]=28;invalid[23]=24;invalid[25]=2;
        bool rejected=false;try{retainedJoints->buildHierarchy(BinaryView(invalid));}catch(const std::exception&){rejected=true;}assert(rejected && retainedJoints->root==root);}
    auto* savedTreeCalc=J3DJoint::mCurrentMtxCalc;Vec savedCurrentScale=J3DSys::mCurrentS;
    J3DJoint::mCurrentMtxCalc=&core;retainedTree.setBasicMtxCalc(&core);size_t hierarchyMatrices=0;
    std::unique_ptr<NativeEnvelopeResource> envelopeResource;
    for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="EVP1")envelopeResource=std::make_unique<NativeEnvelopeResource>(section,rig.joints.size());}
    assert(envelopeResource && envelopeResource->counts.size()==rig.envelopes.size());
    J3DJointTree skinTree;envelopeResource->bind(skinTree);buffer.mJointTree=&skinTree;
    std::vector<u8> envelopeFlags(rig.envelopes.size());std::vector<std::array<float,12>> envelopeMatrices(rig.envelopes.size());
    buffer.mpEvlpScaleFlagArr=envelopeFlags.data();buffer.mpWeightEvlpMtx=reinterpret_cast<Mtx*>(envelopeMatrices.data());
    std::unique_ptr<NativeDrawMatrixResource> drawResource;for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="DRW1")drawResource=std::make_unique<NativeDrawMatrixResource>(section,rig.joints.size(),rig.envelopes.size());}assert(drawResource);
    skinTree.mDrawMtxData=drawResource->data;for(auto* shape:shapePointers)shape->setDrawMtxDataPointer(&drawResource->data);
    std::vector<std::array<float,12>> drawMatrices(drawResource->data.mEntryNum);Mtx* viewDraw=reinterpret_cast<Mtx*>(drawMatrices.data());buffer.mpDrawMtxArr[1]=&viewDraw;
    Mtx savedView;std::memcpy(savedView,j3dSys.getViewMtx(),sizeof(Mtx));const float camera[3][4]={{1,0,0,-5},{0,0,-1,2},{0,1,0,8}};std::memcpy(j3dSys.getViewMtx(),camera,sizeof(Mtx));
    std::vector<std::array<float,9>> normalMatrices(drawResource->data.mEntryNum);Mtx33* viewNormals=reinterpret_cast<Mtx33*>(normalMatrices.data());buffer.mpNrmMtxArr[1]=&viewNormals;
    std::vector<u8> drawScaleFlags(drawResource->data.mEntryNum);u32 shapeView=0;
    for(auto* shape:shapePointers){shape->setDrawMtx(&viewDraw);shape->setNrmMtx(&viewNormals);shape->setCurrentViewNoPtr(&shapeView);shape->setScaleFlagArray(drawScaleFlags.data());}
    Native::gpuResources().addNativeWords(drawMatrices.data(),drawMatrices.size()*sizeof(drawMatrices[0]));Native::gpuResources().addNativeWords(normalMatrices.data(),normalMatrices.size()*sizeof(normalMatrices[0]));
    auto retailAttribute=[&](unsigned attr,u16 index){const GXVtxAttrFmtList* format=nullptr;for(auto& candidate:retainedVertices->formats)if(unsigned(candidate.attr)==attr)format=&candidate;assert(format);
        unsigned type=unsigned(format->type),width=type<=1?1:type<=3?2:4,components=attr==10?3:unsigned(format->cnt)+1;
        const auto* array=static_cast<const u8*>(attr==10?retainedVertices->data.mVtxNrmArray:retainedVertices->data.mVtxTexCoordArray[0]);size_t offset=array-retainedVertices->storage.data();BinaryView raw(retainedVertices->storage);std::array<float,3> result{};
        for(unsigned k=0;k<components;++k){size_t p=offset+size_t(index)*width*components+k*width;
            if(type==4)result[k]=raw.f32(p);else{int value=width==1?(type==1?int(int8_t(raw.u8(p))):int(raw.u8(p))):(type==3?int(int16_t(raw.u16(p))):int(raw.u16(p)));unsigned fraction=attr==10?(type==1?6:14):unsigned(format->frac);result[k]=float(double(value)/std::pow(2.0,int(fraction)));}}
        return result;
    };
    size_t normalUvVertices=0;size_t geometryTriangles=0;size_t geometryVertices=0;size_t shapeMatrixLoads=0;size_t normalChecks=0;size_t drawChecks=0;
    size_t skinMatrices=0;
    for(unsigned tick=0;tick<30;++tick){animation->mFrame=float(tick);
        Vec modelScale=tick%2?Vec{2,3,4}:Vec{1,1,1};
        std::vector<std::array<float,12>> expected(rig.joints.size());std::vector<TVec3f> scales(rig.joints.size());
        for(size_t i=0;i<rig.joints.size();++i){J3DMtxCalc::setJoint(ownedJoints[i].get());TVec3f translation;core.calcSingle(&scales[i],&translation);
            auto local=buffer.getAnmMtx(int(i));float parent[3][4]={{modelScale.x,0,0,4},{0,modelScale.y,0,-2},{0,0,modelScale.z,9}};
            int pi=rig.joints[i].parent;assert(pi<int(i));if(pi>=0)std::memcpy(parent,expected[pi].data(),sizeof(Mtx));
            for(unsigned r=0;r<3;++r)for(unsigned c=0;c<4;++c){float value=c==3?parent[r][3]:0;
                for(unsigned k=0;k<3;++k){float entry=c==3?(&translation.x)[k]:local[k][c]*(&scales[i].x)[c];
                    if(c<3 && pi>=0 && rig.joints[i].scaleCompensate)entry/=(&scales[pi].x)[k];value+=parent[r][k]*entry;}
                expected[i][r*4+c]=value;}
        }
        const float base[3][4]={{1,0,0,4},{0,1,0,-2},{0,0,1,9}};std::memcpy(J3DSys::mCurrentMtx,base,sizeof(Mtx));
        J3DSys::mParentS={7,8,9};J3DSys::mCurrentS={2,3,4};
        retainedTree.calc(&buffer,modelScale,base);
        for(size_t i=0;i<rig.joints.size();++i){auto actual=buffer.getAnmMtx(int(i));for(unsigned r=0;r<3;++r)for(unsigned c=0;c<4;++c){float value=expected[i][r*4+c];assert(std::fabs(actual[r][c]-value)<0.0002f*(1+std::fabs(value)));}++hierarchyMatrices;}
        for(auto* material:materialPointers){auto* materialJoint=material->getJoint();assert(materialJoint);material->calc(buffer.getAnmMtx(materialJoint->getJntNo()));}
        buffer.calcWeightEnvelopeMtx();
        for(size_t i=0;i<rig.envelopes.size();++i){u8 flag=1;
            for(auto entry:rig.envelopes[i])flag&=scaleFlags[entry.joint];assert(envelopeFlags[i]==flag);
            for(unsigned r=0;r<3;++r)for(unsigned c=0;c<4;++c){double value=0;
                for(auto entry:rig.envelopes[i]){auto& world=expected[entry.joint];auto& inverse=rig.inverseBind[entry.joint];double composed=c==3?world[r*4+3]:0;
                    for(unsigned k=0;k<3;++k)composed+=double(world[r*4+k])*inverse[k*4+c];value+=composed*entry.weight;}
                assert(std::fabs(envelopeMatrices[i][r*4+c]-value)<0.0005*(1+std::fabs(value)));}++skinMatrices;
        }
        buffer.calcDrawMtx(0,modelScale,base);
        for(size_t i=0;i<drawResource->data.mEntryNum;++i){auto entry=rig.drawMatrices[i];auto& source=entry.weighted?envelopeMatrices[entry.index]:expected[entry.index];
            for(unsigned r=0;r<3;++r)for(unsigned c=0;c<4;++c){float value=c==3?camera[r][3]:0;for(unsigned k=0;k<3;++k)value+=camera[r][k]*source[k*4+c];if(!(std::fabs(drawMatrices[i][r*4+c]-value)<0.0005f*(1+std::fabs(value)))){std::printf("mismatch tick%u draw%zu weighted%d index%u rigid%u row%u col%u actual%g expected%g view%u\n",tick,i,int(entry.weighted),entry.index,drawResource->data.mDrawFullWgtMtxNum,r,c,drawMatrices[i][r*4+c],value,buffer.mCurrentViewNo);std::abort();}}++drawChecks;}
        buffer.calcNrmMtx();
        for(size_t i=0;i<drawResource->data.mEntryNum;++i){auto entry=rig.drawMatrices[i];u8 flag=entry.weighted?envelopeFlags[entry.index]:scaleFlags[entry.index];
            for(unsigned r=0;r<3;++r)for(unsigned c=0;c<3;++c){if(flag)assert(normalMatrices[i][r*3+c]==drawMatrices[i][r*4+c]);
                else{double dot=0;for(unsigned k=0;k<3;++k)dot+=double(normalMatrices[i][k*3+r])*drawMatrices[i][k*4+c];assert(std::fabs(dot-(r==c?1:0))<0.001);}}++normalChecks;}
        for(size_t i=0;i<drawScaleFlags.size();++i){auto entry=rig.drawMatrices[i];drawScaleFlags[i]=entry.weighted?envelopeFlags[entry.index]:scaleFlags[entry.index];}
        Native::gpuResources().refresh(drawMatrices.data(),drawMatrices.size()*sizeof(drawMatrices[0]));Native::gpuResources().refresh(normalMatrices.data(),normalMatrices.size()*sizeof(normalMatrices[0]));
        size_t packetIndex=0;for(auto* shape:shapePointers){alignas(32) u8 commands[8192]{};GDLObj output;GDInitGDLObj(&output,commands,sizeof(commands));GDSetCurrent(&output);shape->setArrayAndBindPipeline();std::array<u16,10> slotIndices;slotIndices.fill(0xffff);
            for(unsigned group=0;group<shape->mMtxGroupNum;++group){auto* matrix=shape->mShapeMtx[group];size_t offset=GDGetCurrOffset();matrix->load();unsigned actual=GDGetCurrOffset()-unsigned(offset);unsigned expected=0;
                for(u16 slot=0;slot<matrix->getUseMtxNum();++slot){u16 index=matrix->getUseMtxIndex(slot);if(index==0xffff)continue;assert(index<drawResource->data.mEntryNum);
                    const u8* pair=commands+offset+expected;assert(pair[0]==0x20 && (u16(pair[1])<<8|pair[2])==index);assert(pair[5]==0x28 && (u16(pair[6])<<8|pair[7])==index);slotIndices[slot]=index;expected+=10;++shapeMatrixLoads;}
                assert(actual==expected);Native::GxCommandState loadedState;loadedState.consume(static_cast<const u8*>(shape->getVcdVatCmd()),J3DShape::kVcdVatDLSize,&Native::gpuResources());loadedState.consume(commands,GDGetCurrOffset(),&Native::gpuResources());
                                auto* draw=shape->mShapeDraw[group];alignas(32) u8 drawCall[32]{};GDLObj callOutput;GDInitGDLObj(&callOutput,drawCall,sizeof(drawCall));GDSetCurrent(&callOutput);draw->draw();unsigned callSize=GDGetCurrOffset();GDSetCurrent(&output);
                assert(callSize==9 && be32(drawCall+1)==Native::gpuResources().byteAddress(draw->getDisplayList()) && be32(drawCall+5)==draw->getDisplayListSize());
                std::vector<std::array<u32,3>> triangles;auto decodedVertices=decodeCalledShapePositions(loadedState,drawCall,callSize,Native::gpuResources(),&triangles);
                assert(packetIndex<referenceMesh.packets.size());auto& packet=referenceMesh.packets[packetIndex++];assert(packet.shape==shape->getIndex() && packet.vertices.size()==decodedVertices.size());assert(triangles==packet.triangles);geometryTriangles+=triangles.size();
                for(size_t vi=0;vi<decodedVertices.size();++vi){auto& vertex=decodedVertices[vi];assert(vertex.hasNormal && vertex.hasTexCoord0);assert(vertex.normalIndex==packet.vertices[vi].attributeIndices[10] && vertex.texCoordIndex==packet.vertices[vi].attributeIndices[13]);auto sourceNormal=retailAttribute(10,vertex.normalIndex),sourceUv=retailAttribute(13,vertex.texCoordIndex);for(unsigned k=0;k<3;++k){assert(vertex.normal[k]==sourceNormal[k]);assert(vertex.texCoord0[k]==sourceUv[k]);}float length=0;for(float component:vertex.normal)length+=component*component;assert(length>0.8f && length<1.2f);++normalUvVertices;}                assert(!decodedVertices.empty());for(auto vertex:decodedVertices){unsigned slot=vertex.matrix/3;assert(vertex.matrix%3==0 && slot<10 && slotIndices[slot]!=0xffff);auto& normalReference=normalMatrices[slotIndices[slot]];for(unsigned row=0;row<3;++row){float value=0;for(unsigned k=0;k<3;++k)value+=normalReference[row*3+k]*vertex.normal[k];assert(std::fabs(vertex.transformedNormal[row]-value)<0.0001f*(1+std::fabs(value)));}auto& reference=drawMatrices[slotIndices[slot]];auto position=referenceModel.positions[vertex.positionIndex];
                    for(unsigned row=0;row<3;++row){float value=reference[row*4+3];for(unsigned k=0;k<3;++k)value+=reference[row*4+k]*position[k];assert(std::fabs(vertex.position[row]-value)<0.0005f*(1+std::fabs(value)));}}
                geometryVertices+=decodedVertices.size();
                for(u16 slot=0;slot<matrix->getUseMtxNum();++slot){u16 index=matrix->getUseMtxIndex(slot);if(index==0xffff)continue;
                    for(unsigned k=0;k<12;++k){u32 bits;std::memcpy(&bits,&drawMatrices[index][k],4);assert(loadedState.xf(slot*12+k)==bits);}
                    for(unsigned k=0;k<9;++k){u32 bits;std::memcpy(&bits,&normalMatrices[index][k],4);assert(loadedState.xf(0x400+slot*9+k)==bits);}}
            }
            unsigned commandSize=GDGetCurrOffset();GDSetCurrent(nullptr);Native::GxCommandState matrixState;matrixState.consume(commands,commandSize,&Native::gpuResources());}
        if(tick==0){Native::GxCommandState streamState;size_t packets=0,vertices=0;J3DShape::resetVcdVatCache();
            for(auto* shape:shapePointers){streamState=Native::GxCommandState{};J3DShape::resetVcdVatCache();NativeMaterialCommands materialCommands(*shape->getMaterial(),heap);streamState.consume(materialCommands.bytes().data(),materialCommands.bytes().size(),&Native::gpuResources());assert(streamState.tevStageCount()==materialCommands.state().tevStageCount());for(unsigned reg=0;reg<256;++reg)assert(streamState.bp(reg)==materialCommands.state().bp(reg));alignas(32) u8 stream[8192]{};GDLObj streamOutput;GDInitGDLObj(&streamOutput,stream,sizeof(stream));GDSetCurrent(&streamOutput);shape->drawFast();unsigned length=GDGetCurrOffset();GDSetCurrent(nullptr);
                replayShapeStream(streamState,stream,length,Native::gpuResources(),[&](const auto& decoded,const auto& triangles){auto& reference=referenceMesh.packets.at(packets++);assert(decoded.size()==reference.vertices.size() && triangles==reference.triangles);                    for(size_t i=0;i<decoded.size();++i){auto& input=reference.vertices[i];assert(decoded[i].positionIndex==input.position);auto& draw=drawMatrices[input.drawMatrix];auto point=referenceModel.positions[input.position];
                        for(unsigned row=0;row<3;++row){float value=draw[row*4+3];for(unsigned col=0;col<3;++col)value+=draw[row*4+col]*point[col];assert(std::fabs(decoded[i].position[row]-value)<0.0005f*(1+std::fabs(value)));}}
                    vertices+=decoded.size();});}
            assert(packets==referenceMesh.packets.size() && vertices==referenceMesh.vertices);std::printf("PASS original material plus drawFast stream: %zu geometry packets, %zu vertices\n",packets,vertices);
        }
        const float scaledBase[3][4]={{modelScale.x,0,0,4},{0,modelScale.y,0,-2},{0,0,modelScale.z,9}};
        assert(std::memcmp(J3DSys::mCurrentMtx,scaledBase,sizeof(Mtx))==0);assert(J3DSys::mParentS.x==1 && J3DSys::mCurrentS.x==modelScale.x);
    }
    std::printf("PASS original matrix buffer: %zu retail animated skinning envelopes\n",skinMatrices);
    std::memcpy(j3dSys.getViewMtx(),savedView,sizeof(Mtx));std::printf("PASS original draw matrices: %zu camera/rigid/envelope compositions\n",drawChecks);
    std::printf("PASS original normal matrices: %zu copies/inverse-transpose checks\n",normalChecks);
    std::printf("PASS original shape geometry: %zu transformed vertices, %zu independently matched triangles\n",geometryVertices,geometryTriangles);
    j3dSys.setVtxPos(previousPositions);j3dSys.setVtxNrm(previousNormals);j3dSys.setVtxCol(static_cast<GXColor*>(previousColors));
    std::printf("PASS original normal/UV decoding: %zu retail vertex attribute indices and unit normals\n",normalUvVertices);
    const u32 commandAddress=Native::gpuResources().byteAddress(retainedShapes->commands.get());
    const u32 geometryAddress=Native::gpuResources().byteAddress(retainedShapes->tables.displayLists.data());
    retainedShapes.reset();
    for(u32 address:{commandAddress,geometryAddress}){bool rejected=false;try{Native::gpuResources().resolve(address,1);}catch(const std::exception&){rejected=true;}assert(rejected);}
    std::printf("PASS retained shape GPU lifecycle: repeated command preparation and disposal of commands/geometry\n");
    j3dSys.setTexture(previousTexture);Native::gpuResources().remove(retainedTextures->storage.data());

    Native::gpuResources().remove(drawMatrices.data());Native::gpuResources().remove(normalMatrices.data());
    std::printf("PASS original shape matrix loads: %zu position/normal indexed command pairs\n",shapeMatrixLoads);
    J3DJoint::mCurrentMtxCalc=savedTreeCalc;J3DSys::mCurrentS=savedCurrentScale;
    std::printf("PASS original joint recursion: %zu retail Mario hierarchy matrices and parent state restoration\n",hierarchyMatrices);
    std::memcpy(J3DSys::mCurrentMtx,savedCurrent,sizeof(Mtx));J3DSys::mParentS=savedParent;j3dSys.mCurrentMtxCalc=savedCalc;
    std::printf("PASS original core calc: %zu parent matrix compositions, scale compensation and flags\n",worldMatrices);
    {auto temporary=modelBytes;auto retainedModel=std::make_unique<NativeModelResource>(temporary);temporary.clear();temporary.shrink_to_fit();
        assert(retainedModel->tree.getJointNum()==30 && retainedModel->shapes->pointers.size()==9 && retainedModel->materials.size()==9);
        J3DModelData originalModel;retainedModel->bind(originalModel);assert(originalModel.getJointNum()==30 && originalModel.getMaterialNum()==9 && originalModel.getShapeNum()==9);assert(originalModel.getTexture()==retainedModel->textures.get());for(unsigned i=0;i<9;++i)assert(originalModel.getShapeNodePointer(i)==retainedModel->shapes->pointers[i] && originalModel.getMaterialNodePointer(i)==retainedModel->materialPointers[i]);
        assert(originalModel.getVtxNum()==referenceModel.positions.size());
        for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="INF1")assert(originalModel.getVertexData().mPacketNum==section.u32(12));
            if(section.tag(0)=="VTX1"){size_t end=section.size();for(unsigned field:{20u,24u,32u})if(section.u32(field)){end=section.u32(field);break;}
                unsigned type=4;for(size_t p=section.u32(8);section.u32(p)!=255;p+=16)if(section.u32(p)==10)type=section.u32(p+8);assert(unsigned(originalModel.getVertexData().getVtxNrmType())==type);assert(originalModel.getNrmNum()==(end-section.u32(16))/(type==4?12:6)+1);}}
        for(auto format:retainedVertices->formats){if(format.attr==GX_VA_POS)assert(originalModel.getVertexData().getVtxPosType()==format.type && originalModel.getVertexData().getVtxPosFrac()==format.frac);}
        for(auto names:{originalModel.getMaterialName(),originalModel.getTextureName()}){assert(names && names->mNameNum);for(u16 i=0;i<names->mNameNum;++i){assert(names->getName(i) && names->getIndex(names->getName(i))>=0);}assert(names->getIndex("__missing_native_model_name__")==-1);}
        {J3DTexMtxInfo authored{};
        authored.mProjection=1;authored.mInfo=0;
        authored.mCenter={.25f,.75f,.5f};
        authored.mSRT.mScaleX=2;authored.mSRT.mScaleY=3;authored.mSRT.mRotation=1234;
        authored.mSRT.mTranslationX=-.3f;authored.mSRT.mTranslationY=.7f;
        for(unsigned row=0;row<4;++row)for(unsigned col=0;col<4;++col)authored.mEffectMtx[row][col]=float(row*4+col)+.25f;
        J3DTexMtx copied(authored);auto& actual=copied.getTexMtxInfo();
        assert(actual.mProjection==authored.mProjection && actual.mInfo==authored.mInfo);
        assert(!std::memcmp(&actual.mCenter,&authored.mCenter,sizeof(Vec)));
        assert(actual.mSRT.mScaleX==2 && actual.mSRT.mScaleY==3 && actual.mSRT.mRotation==1234);
        assert(actual.mSRT.mTranslationX==-.3f && actual.mSRT.mTranslationY==.7f);
        assert(!std::memcmp(actual.mEffectMtx,authored.mEffectMtx,sizeof(Mtx44)));
        std::printf("PASS original texture-info assignment: center, SRT and all 16 effect-matrix coefficients\n");}
        auto checkTextureMatrices=[&](const char* phase){
          for(unsigned material=0;material<9;++material)for(unsigned slot=0;slot<8;++slot){
            auto* matrix=originalModel.getMaterialNodePointer(material)->getTexMtx(slot);
            if(!matrix)continue;
            for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col)
              if(!std::isfinite(matrix->getMtx()[row][col])){
                std::fprintf(stderr,"Nonfinite authored matrix phase=%s material=%u slot=%u mode=%u cell=%u,%u\n",phase,material,slot,unsigned(matrix->getTexMtxInfo().mInfo),row,col);
                auto& info=matrix->getTexMtxInfo();std::fprintf(stderr,"SRT scale=%g,%g rotation=%d translation=%g,%g center=%g,%g,%g flags=%x block=%x\n",info.mSRT.mScaleX,info.mSRT.mScaleY,int(info.mSRT.mRotation),info.mSRT.mTranslationX,info.mSRT.mTranslationY,info.mCenter.x,info.mCenter.y,info.mCenter.z,j3dSys.mFlags,originalModel.getMaterialNodePointer(material)->getTexGenBlock()->getType());
                std::abort();
              }
          }
        };
        for(auto* material:retainedModel->materialPointers)for(unsigned slot=0;slot<8;++slot)
          if(auto* matrix=material->getTexMtx(slot))std::memset(matrix->getMtx(),0xff,sizeof(Mtx));
        Mtx materialIdentity;PSMTXIdentity(materialIdentity);for(auto* material:retainedModel->materialPointers)material->calc(materialIdentity);
        checkTextureMatrices("identity calc");
        retainedModel->prepareMaterialLists(heap);assert(retainedModel->materialLists.size()==9);
        std::vector<u32> materialAddresses;
        for(auto& owner:retainedModel->materialLists){owner->ensureDoubleBuffer();auto& list=owner->list;assert(list.mpDisplayList[0]!=list.mpDisplayList[1]);assert(!std::memcmp(list.mpDisplayList[0],list.mpDisplayList[1],list.mMaxSize));for(auto* buffer:list.mpDisplayList)materialAddresses.push_back(Native::gpuResources().byteAddress(buffer));}
        for(auto* material:retainedModel->materialPointers){auto* list=material->getSharedDisplayListObj();assert(list && list->mSize);Native::GxCommandState state;state.consume(static_cast<const u8*>(list->mpDisplayList[0]),list->mSize,&Native::gpuResources());assert(state.tevStageCount()>0);}
        std::printf("PASS retained original material display lists: 9 persistent registered lists\n");
        {NativeMatrixBufferResource allocation(originalModel,2);auto& allocated=allocation.buffer;
        assert(allocated.mJointTree==&originalModel.getJointTree() && allocated.mpAnmMtx && allocated.mpWeightEvlpMtx && allocated.mpScaleFlagArr && allocated.mpEvlpScaleFlagArr);
        {alignas(J3DMatPacket) unsigned char poisoned[sizeof(J3DMatPacket)];
        std::memset(poisoned,0xff,sizeof(poisoned));
        auto* packet=::new(static_cast<void*>(poisoned)) J3DMatPacket();
        assert(packet->getShapePacket()==nullptr);
        packet->~J3DMatPacket();
        std::printf("PASS native material packet: shape-list initialization over poisoned storage\n");}
        {NativeModelPackets packets(originalModel,allocation);auto& model=packets.model;
        for(unsigned i=0;i<9;++i){assert(model.getShapePacket(i)->getShape()==originalModel.getShapeNodePointer(i));assert(model.getMatPacket(i)->getMaterial()==originalModel.getMaterialNodePointer(i));}
        auto* previousModel=j3dSys.getModel();auto* priorCalc=J3DJoint::mCurrentMtxCalc;auto* priorBuffer=J3DMtxCalc::getMtxBuffer();auto* priorJoint=J3DMtxCalc::getJoint();
        Mtx priorCurrent;std::memcpy(priorCurrent,J3DSys::mCurrentMtx,sizeof(Mtx));Vec priorParent=J3DSys::mParentS,priorScale=J3DSys::mCurrentS;
        originalModel.setBasicMtxCalc(&core);model.mBaseScale={2,3,4};PSMTXIdentity(model.mBaseTransformMtx);model.mBaseTransformMtx[0][3]=4;model.mBaseTransformMtx[1][3]=-2;model.mBaseTransformMtx[2][3]=9;
        animation->mFrame=29;model.calc();checkTextureMatrices("model calc");
        for(size_t i=0;i<storageMatrices.size();++i)for(unsigned k=0;k<12;++k)assert(std::fabs(reinterpret_cast<float*>(allocated.mpAnmMtx[i])[k]-storageMatrices[i][k])<0.0001f);
        for(size_t i=0;i<envelopeMatrices.size();++i)for(unsigned k=0;k<12;++k)assert(std::fabs(reinterpret_cast<float*>(allocated.mpWeightEvlpMtx[i])[k]-envelopeMatrices[i][k])<0.0001f);
        {NativeMatrixBufferResource mirrorAllocation(originalModel,2);
        NativeModelPackets mirrorPackets(originalModel,mirrorAllocation);
        auto& mirror=mirrorPackets.model;
        MR::copyJointAnimation(&mirror,&model);
        for(size_t i=0;i<storageMatrices.size();++i)
            assert(!std::memcmp(mirror.getAnmMtx(i),model.getAnmMtx(i),sizeof(Mtx)));
        for(size_t i=0;i<envelopeMatrices.size();++i)
            assert(!std::memcmp(mirror.getWeightAnmMtx(i),model.getWeightAnmMtx(i),sizeof(Mtx)));
        MR::syncMaterialAnimation(&mirror,&model);
        for(unsigned i=0;i<9;++i)
            assert(mirror.getShapePacket(i)->getDisplayListObj()==model.getShapePacket(i)->getDisplayListObj());
        mirror.getAnmMtx(0)[0][3]+=17;
        assert(mirror.getAnmMtx(0)[0][3]!=model.getAnmMtx(0)[0][3]);
        std::printf("PASS original mirror synchronization: retail joint/envelope copies, shared material lists, independent matrix storage\n");
        }
        model.prepareShapePackets(); // Restore shared shape bindings after the second model fixture.
        Mtx priorView;std::memcpy(priorView,j3dSys.getViewMtx(),sizeof(Mtx));
        for(unsigned view=0;view<2;++view){allocated.mCurrentViewNo=view;std::memcpy(j3dSys.getViewMtx(),camera,sizeof(Mtx));for(unsigned row=0;row<3;++row)j3dSys.getViewMtx()[row][3]+=float(view*3);
            model.viewCalc();allocation.refreshGpuMatrices();auto* actualDraw=model.getDrawMtxPtr();auto* actualNormal=model.getNrmMtxPtr();u32 matrixAddress=Native::gpuResources().byteAddress(actualDraw);auto* matrixBytes=Native::gpuResources().resolve(matrixAddress,sizeof(Mtx));for(unsigned k=0;k<12;++k){u32 bits;std::memcpy(&bits,reinterpret_cast<float*>(actualDraw)+k,4);assert(be32(matrixBytes+k*4)==bits);}
            for(size_t i=0;i<drawMatrices.size();++i){for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col){float expected=drawMatrices[i][row*4+col]+(col==3?float(view*3):0);assert(std::fabs(actualDraw[i][row][col]-expected)<0.0001f);}
                for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col)assert(std::fabs(actualNormal[i][row][col]-normalMatrices[i][row*3+col])<0.0001f);}}
        retainedModel->vertices->bindCurrentArrays();retainedModel->shapes->prepareVertexCommands(originalModel.getVertexData());
        std::vector<NativeShapeVertex> gpuVertices;std::vector<std::array<u32,3>> gpuTriangles;Native::GxCommandState packetState;const u8 initialColorUpdate[]={0x61,0x41,0,0,24};packetState.consume(initialColorUpdate,sizeof(initialColorUpdate));size_t packetCount=0,packetVertices=0;J3DShape::resetVcdVatCache();
        struct MaterialBatch{Native::GxCommandState state;size_t first,count;};std::vector<MaterialBatch> materialBatches;
        std::ofstream materialAudit(std::filesystem::path(argv[0]).parent_path()/"retail-mario-material-audit.txt");assert(materialAudit.good());
        auto* previousShapePacket=j3dSys.getShapePacket();
        for(unsigned index=0;index<9;++index){size_t referenceIndex=0;unsigned shapeIndex=originalModel.getMaterialNodePointer(index)->getShape()->getIndex();for(unsigned previous=0;previous<shapeIndex;++previous)referenceIndex+=originalModel.getShapeNodePointer(previous)->mMtxGroupNum;
            alignas(32) u8 stream[8192]{};GDLObj output;GDInitGDLObj(&output,stream,sizeof(stream));GDSetCurrent(&output);model.getMatPacket(index)->draw();unsigned length=GDGetCurrOffset();GDSetCurrent(nullptr);
            replayShapeStream(packetState,stream,length,Native::gpuResources(),[&](const auto& decoded,const auto& triangles){
 materialBatches.push_back({packetState,gpuTriangles.size(),triangles.size()});
 materialAudit<<" vertexColor0Present "<<decoded.front().hasColor[0]<<" RGBA "<<decoded.front().colors[0][0]<<","<<decoded.front().colors[0][1]<<","<<decoded.front().colors[0][2]<<","<<decoded.front().colors[0][3]<<"\n";
 materialAudit<<"material "<<index<<" packet "<<packetCount<<" stages "<<packetState.tevStageCount()<<" dual "<<packetState.xf(0x1012)<<" texgenCount "<<packetState.xf(0x103f)<<"\n";
 try{
  auto rgb=packetState.lightingChannel(0),alpha=packetState.lightingChannel(0,true);
  materialAudit<<" lighting rgb enabled "<<rgb.enabled<<" vertexMaterial "<<rgb.materialVertex<<" vertexAmbient "<<rgb.ambientVertex<<" mask "<<rgb.lightMask<<" diffuse "<<rgb.diffuse<<" attenuation "<<rgb.attenuation<<" alphaMask "<<alpha.lightMask<<"\n";
  auto lit=Native::evaluateDiffuseLighting(packetState,0,decoded.front().position,decoded.front().transformedNormal,decoded.front().colors[0]);
  materialAudit<<" lighting evaluated "<<lit[0]<<","<<lit[1]<<","<<lit[2]<<","<<lit[3]<<"\n";
 }catch(const std::exception& error){materialAudit<<" lighting unavailable: "<<error.what()<<"\n";}
 for(unsigned stage=0;stage<packetState.tevStageCount();++stage){
  auto order=packetState.tevOrder(stage);auto indirect=packetState.tevIndirect(stage);
  materialAudit<<" stage "<<stage<<" enabled "<<order.enabled<<" texture "<<order.texture<<" coord "<<order.coordinate<<" raster "<<order.channel<<" indirectMatrix "<<indirect.matrix<<"\n";
  if(order.enabled){
   materialAudit<<"  xf "<<packetState.xf(0x1040+order.coordinate)<<" post "<<packetState.xf(0x1050+order.coordinate)<<" selector "<<unsigned(decoded.front().textureMatrices[order.coordinate])<<"\n";
   try{auto uv=generateModelTexCoord(packetState,decoded.front(),order.coordinate);materialAudit<<"  generated "<<uv[0]<<","<<uv[1]<<","<<uv[2]<<"\n";}
   catch(const std::exception& error){materialAudit<<"  unsupported: "<<error.what()<<"\n";}
  }
 }
 u32 first=u32(gpuVertices.size());gpuVertices.insert(gpuVertices.end(),decoded.begin(),decoded.end());for(auto triangle:triangles){for(auto& index:triangle)index+=first;gpuTriangles.push_back(triangle);}auto& reference=referenceMesh.packets.at(referenceIndex++);++packetCount;assert(decoded.size()==reference.vertices.size() && triangles==reference.triangles);
                for(size_t i=0;i<decoded.size();++i){auto& vertex=reference.vertices[i];auto* draw=model.getDrawMtxPtr();auto point=referenceModel.positions[vertex.position];
                    for(unsigned row=0;row<3;++row){float expected=draw[vertex.drawMatrix][row][3];for(unsigned col=0;col<3;++col)expected+=draw[vertex.drawMatrix][row][col]*point[col];assert(std::fabs(decoded[i].position[row]-expected)<0.0005f*(1+std::fabs(expected)));}}packetVertices+=decoded.size();});}
        assert(packetCount==referenceMesh.packets.size() && packetVertices==referenceMesh.vertices);j3dSys.setShapePacket(previousShapePacket);
        {
        std::array<float,3> low=gpuVertices.front().position,high=low;
        for(const auto& vertex:gpuVertices)for(unsigned axis=0;axis<3;++axis){low[axis]=std::min(low[axis],vertex.position[axis]);high[axis]=std::max(high[axis],vertex.position[axis]);}
        float extent=0;for(unsigned axis=0;axis<3;++axis)extent=std::max(extent,high[axis]-low[axis]);
        assert(extent>0);
        for(auto& vertex:gpuVertices){for(unsigned axis=0;axis<3;++axis)vertex.position[axis]-=(low[axis]+high[axis])*.5f;vertex.position[2]-=extent*2;}
        TProj3f projection;projection.makePerspective(45,1,1,extent*10);
        auto triangles=projectModelTriangles(gpuVertices,gpuTriangles,projection,{1,1,1,1});
        Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
        Native::D3dEfb target(*context.Get(),256,256);Native::D3dModelDraw renderer(*context.Get());float black[4]={0,0,0,1};target.clear(black,1);
        for(const auto& batch:materialBatches){
         std::array<std::unique_ptr<NativeModelTexture>,8> uploaded;
         std::array<ID3D11ShaderResourceView*,8> views{};std::array<ID3D11SamplerState*,8> samplers{};
         for(unsigned stage=0;stage<batch.state.tevStageCount();++stage){
          auto order=batch.state.tevOrder(stage);if(!order.enabled||uploaded[order.texture])continue;
          auto image=decodeCommandTexture(batch.state,order.texture,Native::gpuResources());
          // Diagnostic currently decodes command base level only.
          image.minFilter=image.minFilter==1||image.minFilter==3||image.minFilter==5?1:0;
          uploaded[order.texture]=std::make_unique<NativeModelTexture>(*device.Get(),image);
          views[order.texture]=uploaded[order.texture]->view.Get();samplers[order.texture]=uploaded[order.texture]->sampler.Get();
         }
         renderer.setMaterial(batch.state,views,samplers);
         std::vector<std::array<u32,3>> indices(gpuTriangles.begin()+batch.first,gpuTriangles.begin()+batch.first+batch.count);
         auto projected=projectModelTriangles(gpuVertices,indices,projection,{1,1,1,1},&batch.state);
         renderer.draw(target,projected);
        }
        auto pixels=target.readColor();size_t visible=0;for(size_t pixel=0;pixel<pixels.size();pixel+=4)visible+=pixels[pixel]!=0;assert(visible>100&&visible<256*256);
                {
        std::vector<u8> image(54+256*256*4,0);image[0]='B';image[1]='M';
        auto word=[&](size_t offset,u32 value){for(unsigned byte=0;byte<4;++byte)image[offset+byte]=u8(value>>(byte*8));};
        word(2,u32(image.size()));word(10,54);word(14,40);word(18,256);word(22,256);image[26]=1;image[28]=32;word(34,256*256*4);
        for(unsigned row=0;row<256;++row)for(unsigned col=0;col<256;++col){
         const size_t src=(size_t(255-row)*256+col)*4,dst=54+(size_t(row)*256+col)*4;
         image[dst]=pixels[src+2];image[dst+1]=pixels[src+1];image[dst+2]=pixels[src];image[dst+3]=255;
        }
        std::ofstream output(std::filesystem::path(argv[0]).parent_path()/"retail-mario-materials.bmp",std::ios::binary);
        output.write(reinterpret_cast<const char*>(image.data()),std::streamsize(image.size()));assert(output.good());
        }
std::printf("PASS retail Mario native GPU mip-texture material diagnostic: %zu triangles, %zu visible diagnostic pixels\n",gpuTriangles.size(),visible);
        }
        std::printf("PASS original material packet draw: %zu packets, %zu transformed vertices\n",packetCount,packetVertices);
        {J3DDrawBuffer first(32),second(32);auto* oldFirst=j3dSys.mDrawBuffer[0];auto* oldSecond=j3dSys.mDrawBuffer[1];auto* oldTexture=j3dSys.getTexture();j3dSys.mDrawBuffer[0]=&first;j3dSys.mDrawBuffer[1]=&second;
        model.entry();std::vector<size_t> sortedReferences;
        for(auto* drawBuffer:{&first,&second})for(u32 slot=0;slot<drawBuffer->mEntryTableSize;++slot)for(auto* packet=drawBuffer->mpBuffer[slot];packet;packet=packet->getNextPacket()){
            auto* mat=static_cast<J3DMatPacket*>(packet);for(auto* shape=mat->getShapePacket();shape;shape=static_cast<J3DShapePacket*>(shape->getNextPacket())){
                unsigned index=shape->getShape()->getIndex();size_t start=0;for(unsigned prior=0;prior<index;++prior)start+=originalModel.getShapeNodePointer(prior)->mMtxGroupNum;
                for(unsigned group=0;group<shape->getShape()->mMtxGroupNum;++group)sortedReferences.push_back(start+group);}}
        assert(sortedReferences.size()==12);alignas(32) u8 sortedStream[65536]{};GDLObj sortedOutput;GDInitGDLObj(&sortedOutput,sortedStream,sizeof(sortedStream));GDSetCurrent(&sortedOutput);first.draw();second.draw();unsigned sortedLength=GDGetCurrOffset();GDSetCurrent(nullptr);
        Native::GxCommandState sortedState;size_t sortedPackets=0,sortedVertices=0;
        replayShapeStream(sortedState,sortedStream,sortedLength,Native::gpuResources(),[&](const auto& vertices,const auto& triangles){auto& reference=referenceMesh.packets.at(sortedReferences.at(sortedPackets++));assert(vertices.size()==reference.vertices.size() && triangles==reference.triangles);for(size_t i=0;i<vertices.size();++i)assert(vertices[i].positionIndex==reference.vertices[i].position);sortedVertices+=vertices.size();});
        assert(sortedPackets==12 && sortedVertices==6887);j3dSys.mDrawBuffer[0]=oldFirst;j3dSys.mDrawBuffer[1]=oldSecond;j3dSys.setTexture(oldTexture);j3dSys.setShapePacket(previousShapePacket);
        std::printf("PASS original model entry/draw sorting: 12 packets, 6887 vertices through both draw buffers\n");}
        std::memcpy(j3dSys.getViewMtx(),priorView,sizeof(Mtx));
        std::printf("PASS original model viewCalc: two camera transforms, original buffer swaps and draw/normal references\n");
        j3dSys.setModel(previousModel);J3DJoint::mCurrentMtxCalc=priorCalc;J3DMtxCalc::setMtxBuffer(priorBuffer);J3DMtxCalc::setJoint(priorJoint);std::memcpy(J3DSys::mCurrentMtx,priorCurrent,sizeof(Mtx));J3DSys::mParentS=priorParent;J3DSys::mCurrentS=priorScale;
        std::printf("PASS original model calc: retained model animated joint and envelope matrix references\n");}
        std::printf("PASS original model packet setup: 9 shape/material packets with retained display lists\n");
        std::memcpy(allocated.mpAnmMtx,storageMatrices.data(),storageMatrices.size()*sizeof(Mtx));std::memcpy(allocated.mpScaleFlagArr,scaleFlags.data(),scaleFlags.size());
        allocated.calcWeightEnvelopeMtx();
        for(size_t i=0;i<envelopeMatrices.size();++i)for(unsigned k=0;k<12;++k)assert(std::fabs(reinterpret_cast<float*>(allocated.mpWeightEvlpMtx[i])[k]-envelopeMatrices[i][k])<0.0001f);
        for(unsigned side=0;side<2;++side)for(unsigned view=0;view<2;++view)assert(allocated.mpDrawMtxArr[side][view] && allocated.mpNrmMtxArr[side][view]);}
        std::printf("PASS original matrix allocation: two views and double buffers, retained-model envelope references\n");
        assert(retainedModel->tree.getHierarchy()==retainedModel->hierarchyCommands.data());
        size_t hierarchyChecked=0;
        for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="INF1"){
            size_t start=section.u32(20);for(auto command:retainedModel->hierarchyCommands){assert(command.mType==section.u16(start+hierarchyChecked*4) && command.mValue==section.u16(start+hierarchyChecked*4+2));++hierarchyChecked;}}}
        assert(hierarchyChecked && retainedModel->hierarchyCommands.back().mType==0);
        retainedModel->registerGpu();retainedModel->registerGpu();
        const u32 textureAddress=Native::gpuResources().byteAddress(retainedModel->textureTables->storage.data());
        assert(retainedModel->tree.getJointName()->getIndex(retainedModel->tree.getJointName()->getName(0))==0);
        for(auto* shape:retainedModel->shapes->pointers)assert(shape->getMaterial() && shape->getMaterial()->getJoint());
        retainedModel.reset();for(u32 address:materialAddresses){bool rejected=false;try{Native::gpuResources().resolve(address,1);}catch(const std::exception&){rejected=true;}assert(rejected);}bool removed=false;try{Native::gpuResources().resolve(textureAddress,1);}catch(const std::exception&){removed=true;}assert(removed);
        std::printf("PASS retained model assembly: original joint/material/shape graph survives source disposal and releases texture registration\n");}
    {Mtx priorView;std::memcpy(priorView,j3dSys.getViewMtx(),sizeof(Mtx));PSMTXIdentity(j3dSys.getViewMtx());
        Mtx testMatrix={{1,0,0,0},{0,1,0,0},{2,3,4,5}};Vec testPoint={7,11,13};assert(J3DCalcZValue(testMatrix,testPoint)==104);
        J3DDrawBuffer depthBuffer(8);depthBuffer.mZNear=1;depthBuffer.mZFar=81;depthBuffer.mZRatio=10;depthBuffer.setZSort();
        J3DMatPacket packets[5];J3DShapePacket shapes[5];Mtx origin;PSMTXIdentity(origin);depthBuffer.setZMtx(origin);
        const float depths[5]={0,5,20,60,100};for(unsigned i=0;i<5;++i){packets[i].setShapePacket(&shapes[i]);origin[2][3]=-depths[i];assert(depthBuffer.entryZSort(&packets[i])==1);}
        assert(depthBuffer.mpBuffer[7]==&packets[1] && packets[1].getNextPacket()==&packets[0]);assert(depthBuffer.mpBuffer[5]==&packets[2]);assert(depthBuffer.mpBuffer[1]==&packets[3]);assert(depthBuffer.mpBuffer[0]==&packets[4]);
        for(unsigned slot:{2u,3u,4u,6u})assert(!depthBuffer.mpBuffer[slot]);std::memcpy(j3dSys.getViewMtx(),priorView,sizeof(Mtx));
        std::printf("PASS original depth sort: transformed Z, near/far buckets and equal-bucket ordering\n");}
    {auto temporary=modelBytes;auto instance=std::make_unique<NativeModelInstance>(temporary,heap);temporary.clear();temporary.shrink_to_fit();
        auto* priorModel=j3dSys.getModel();auto* priorCalc=J3DJoint::mCurrentMtxCalc;auto* priorBuffer=J3DMtxCalc::getMtxBuffer();auto* priorJoint=J3DMtxCalc::getJoint();
        Mtx transform;PSMTXIdentity(transform);transform[0][3]=4;transform[1][3]=-2;transform[2][3]=9;animation->mFrame=29;
        instance->calculate(core,Vec{2,3,4},transform);instance->calculateView(0);
        for(size_t i=0;i<storageMatrices.size();++i)for(unsigned k=0;k<12;++k)assert(std::fabs(reinterpret_cast<float*>(instance->matrices->buffer.mpAnmMtx[i])[k]-storageMatrices[i][k])<0.0001f);
        auto address=Native::gpuResources().byteAddress(instance->model().getDrawMtxPtr());instance.reset();bool rejected=false;try{Native::gpuResources().resolve(address,1);}catch(const std::exception&){rejected=true;}assert(rejected);
        j3dSys.setModel(priorModel);J3DJoint::mCurrentMtxCalc=priorCalc;J3DMtxCalc::setMtxBuffer(priorBuffer);J3DMtxCalc::setJoint(priorJoint);
        std::printf("PASS combined native model instance: source disposal, original calc/view and GPU cleanup\n");}
    {
     auto data=createNativeModelData(modelBytes.data(),modelBytes.size(),heap);
     std::vector<std::unique_ptr<J3DMaterialAnm>> animators;
     for(unsigned i=0;i<data->getMaterialNum();i++){
      animators.push_back(std::make_unique<J3DMaterialAnm>());
      data->getMaterialNodePointer(i)->mMaterialAnm=animators.back().get();
      assert(data->getMaterialNodePointer(i)->getMaterialAnm()==animators.back().get());
     }
     size_t checked=0;
     for(const auto& file:archive.files){
      auto input=archive.readFile(file.path);if(input.size()<8||std::memcmp(input.data(),"J3D1btp1",8))continue;
      NativeTexturePatternAnimation animation(input.data(),input.size());animation.searchUpdateMaterialID(data.get());
      auto& table=data->getMaterialTable();table.field_0x1c=1;assert(table.entryTexNoAnimator(&animation)==2);table.field_0x1c=0;
      assert(table.entryTexNoAnimator(&animation)==0);
      for(float frame:{0.f,.5f,1.5f,float(animation.mFrameMax)}){
       animation.mFrame=frame;
       for(unsigned i=0;i<data->getMaterialNum();i++)animators[i]->calc(data->getMaterialNodePointer(i));
       for(unsigned track=0;track<animation.getUpdateMaterialNum();track++){
        if(!animation.isValidUpdateMaterialID(track))continue;
        auto* material=data->getMaterialNodePointer(animation.getUpdateMaterialID(track));u16 expected=0;animation.getTexNo(track,&expected);
        assert(material->mTevBlock->getTexNo(animation.mAnmTable[track].mTexNo)==expected);++checked;
       }
      }
      assert(table.removeTexNoAnimator(&animation)==0);
     }
     assert(checked>0);
     for(unsigned i=0;i<data->getMaterialNum();i++)data->getMaterialNodePointer(i)->mMaterialAnm=nullptr;
     std::printf("PASS original retail model BTP attachment, locked table, material calc texture updates and removal: %zu comparisons\n",checked);
    }
    {auto temporary=modelBytes;auto modelData=createNativeModelData(temporary.data(),temporary.size(),heap);temporary.clear();temporary.shrink_to_fit();
        assert(modelData->getJointNum()==30 && modelData->getMaterialNum()==9 && modelData->getShapeNum()==9 && modelData->getTexture());assert(modelData->getMaterialName()->getIndex(modelData->getMaterialName()->getName(0))>=0);
        {auto* previousBuffer=J3DMtxCalc::getMtxBuffer();auto* previousJoint=J3DMtxCalc::getJoint();auto* previousCalc=J3DJoint::mCurrentMtxCalc;
            NativeMatrixBufferResource rest(*modelData);Mtx identity;PSMTXIdentity(identity);modelData->getJointTree().calc(&rest.buffer,Vec{1,1,1},identity);
            for(unsigned i=0;i<30;++i)for(unsigned k=0;k<12;++k)assert(std::isfinite(reinterpret_cast<float*>(rest.buffer.mpAnmMtx[i])[k]));
            unsigned root=modelData->getJointTree().getRootNode()->getJntNo();for(unsigned axis=0;axis<3;++axis)assert(std::fabs(rest.buffer.mpAnmMtx[root][axis][3]-rig.joints[root].translation[axis])<0.0001f);
            J3DMtxCalc::setMtxBuffer(previousBuffer);J3DMtxCalc::setJoint(previousJoint);J3DJoint::mCurrentMtxCalc=previousCalc;}
        auto* important=modelData->getWEvlpImportantMtxIndex();assert(important);unsigned rigid=modelData->getDrawMtxData()->mDrawFullWgtMtxNum;
        for(unsigned i=0;i<rigid;++i)assert(important[i]==rig.drawMatrices[i].index);
        for(size_t i=0;i<rig.envelopes.size();++i){const auto& weights=rig.envelopes[i];auto best=std::max_element(weights.begin(),weights.end(),[](const auto& a,const auto& b){return a.weight<b.weight;});assert(important[rigid+i]==best->joint);}
        auto* list=modelData->getMaterialNodePointer(0)->getSharedDisplayListObj();auto address=Native::gpuResources().byteAddress(list->mpDisplayList[0]);modelData.reset();bool removed=false;try{Native::gpuResources().resolve(address,1);}catch(const std::exception&){removed=true;}assert(removed);
        bool rejected=false;try{createNativeModelData(modelBytes.data(),31,heap);}catch(const std::exception&){rejected=true;}assert(rejected);
        rejected=false;try{createNativeModelData(modelBytes.data(),modelBytes.size()-1,heap);}catch(const std::exception&){rejected=true;}assert(rejected);
        std::printf("PASS owned original model-data factory: source disposal, virtual deletion and truncated inputs\n");}
    {Native::GDCurrentScope restoreWriter;u8 parentBytes[32]{};GDLObj parentWriter;GDInitGDLObj(&parentWriter,parentBytes,sizeof(parentBytes));GDSetCurrent(&parentWriter);
        const u32 flags=J3DMLF_Material_UseIndirect|J3DMLF_UseUniqueMaterials;auto flagged=createNativeModelData(modelBytes.data(),modelBytes.size(),heap,flags);
        assert(__GDCurrentDL==&parentWriter&&GDGetCurrOffset()==0);
        for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="INF1")assert(flagged->getFlag()==(flags|section.u16(8)));}
        assert(flagged->getJointNum()==30 && flagged->getMaterialNum()==9);for(unsigned i=0;i<9;++i){auto* material=flagged->getMaterialNodePointer(i);assert(material->mIndBlock && material->getSharedDisplayListObj());
            if(!material->mPEBlock->getFog()){std::fprintf(stderr,"FAIL retail Mario BDL material %u has no mutable fog block\n",i);return 24;}}
        for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="MAT3"){
            NativeMaterialInitTables tables(modelBytes.data()+block.offset,block.length);J3DMaterialFactory factory(tables);
            for(unsigned i=0;i<9;++i){auto expected=factory.newFog(i);auto* actual=flagged->getMaterialNodePointer(i)->mPEBlock->getFog();
                assert(!std::memcmp(actual->getFogInfo(),expected.getFogInfo(),sizeof(J3DFogInfo)));
                auto changed=*actual->getFogInfo();changed.mStartZ+=17.0f;actual->setFogInfo(changed);assert(actual->getFogInfo()->mStartZ==changed.mStartZ);actual->setFogInfo(*expected.getFogInfo());}
        }}
        std::printf("PASS retail Mario BDL: nine mutable fog blocks match decoded material settings\n");
        std::printf("PASS native model loader flags: original ResourceHolder indirect/unique material options\n");}
    {JKRFileLoader::initializeVolumeList();std::unique_ptr<J3DModelData> fromArchive;
        auto* textureHeap=JKRExpHeap::create(64*1024,&heap,false);assert(textureHeap);ResTIMG* retainedBti=nullptr;u32 retainedTextureAddress=0;
        {auto archiveBytes=disc.readFile("/ObjectData/Mario.arc");assert(nativeRegisterRarc(archiveBytes.data(),archiveBytes.size()));JKRMemArchive mounted;assert(mounted.mountFixed(archiveBytes.data(),JKR_MEM_BREAK_FLAG_0));nativeUnregisterRarc(archiveBytes.data());archiveBytes.clear();archiveBytes.shrink_to_fit();
            auto* resource=mounted.getResource("/mario.bdl");assert(resource);fromArchive=loadNativeArchiveModel(mounted,resource,heap,J3DMLF_Material_UseIndirect|J3DMLF_UseUniqueMaterials);
            retainedBti=loadNativeArchiveBti(mounted,mounted.getResource("/fooline.bti"),textureHeap);
            assert(Native::loadNativeBti(mounted,retainedBti)==retainedBti);
            retainedTextureAddress=Native::gpuResources().address(reinterpret_cast<u8*>(retainedBti)+retainedBti->mImageDataOffset);
            assert(!loadNativeArchiveModel(mounted,nullptr,heap));bool rejected=false;try{loadNativeArchiveModel(mounted,modelBytes.data(),heap);}catch(const std::exception&){rejected=true;}assert(rejected);}
        assert(fromArchive->getJointNum()==30 && fromArchive->getShapeNum()==9 && fromArchive->getMaterialName()->getName(0));fromArchive.reset();assert(JKRFileLoader::sVolumeList.getNumLinks()==0);
        assert(retainedBti->mWidth==64&&retainedBti->mHeight==128);assert(Native::gpuResources().resolve(retainedTextureAddress,64*128*2));
        JKRHeap::destroy(textureHeap);bool removedBti=false;try{Native::gpuResources().resolve(retainedTextureAddress,1);}catch(const std::exception&){removedBti=true;}assert(removedBti);
        std::puts("PASS standalone BTI survives archive release and unregisters pixels on owning-heap destruction");
        std::printf("PASS original archive model adapter: bounded size query, foreign resource rejection and archive-disposal lifetime\n");}
    {auto* cameraHeap=JKRExpHeap::create(64*1024,&heap,false);assert(cameraHeap);
        std::weak_ptr<NativeJMapStorage> lifetime;const char* retained=nullptr;
        {std::vector<u8> disk(60,0);auto put=[&](size_t at,u32 value){for(int i=3;i>=0;--i){disk[at+i]=u8(value);value>>=8;}};
         put(0,1);put(4,1);put(8,28);put(12,32);put(16,1);put(20,0xFFFFFFFF);disk[27]=1;std::memcpy(disk.data()+28,"retained-camera",16);
         JMapInfo temporary;temporary.attachNative(disk);lifetime=temporary.mNativeStorage;
         retained=reinterpret_cast<const char*>(temporary.mData)+28;Native::retainHeapJMap(temporary,cameraHeap);}
        assert(!lifetime.expired()&&std::strcmp(retained,"retained-camera")==0);
        JKRHeap::destroy(cameraHeap);assert(lifetime.expired());
        std::puts("PASS camera string lifetime: decoded table survives reader scope and releases on owning heap teardown");}
    {auto* imageHeap=JKRExpHeap::create(64*1024,&heap,false);assert(imageHeap);
        auto* image=static_cast<u8*>(JKRHeap::alloc(2080,32,imageHeap));assert(image);std::memset(image,0,2080);
        Native::bindHeapGpuRange(image,2080);const auto imageAddress=Native::gpuResources().address(image+32);
        assert(*Native::gpuResources().resolve(imageAddress,2048)==0);
        image[32]=77;Native::gpuResources().refresh(image,2080);assert(*Native::gpuResources().resolve(imageAddress,1)==77);
        u8 rendered=123;Native::gpuResources().write(imageAddress,&rendered,1);assert(image[32]==123);
        bool duplicate=false;try{Native::bindHeapGpuRange(image,2080);}catch(const std::exception&){duplicate=true;}assert(duplicate);
        assert(*Native::gpuResources().resolve(imageAddress,1)==123);
        JKRHeap::destroy(imageHeap);bool removed=false;try{Native::gpuResources().resolve(imageAddress,1);}catch(const std::exception&){removed=true;}assert(removed);
        u8 unowned[32]{};bool rejected=false;try{Native::bindHeapGpuRange(unowned,sizeof(unowned));}catch(const std::exception&){rejected=true;}assert(rejected);
        std::puts("PASS heap-owned dynamic image mapping: CPU refresh, GPU writes, duplicate rejection and teardown cleanup");}
    {alignas(32) u8 original[32]={0xa0,0,3,0,0,1,3,0,2,6,0,3};J3DShapeDraw rewrite(original,sizeof(original));assert(rewrite.countVertex(3)==3);
        rewrite.addTexMtxIndexInDL(3,1,30);auto* bytes=static_cast<const u8*>(rewrite.getDisplayList());assert(bytes[0]==0xa0 && bytes[1]==0 && bytes[2]==3);
        for(unsigned i=0;i<3;++i){assert(bytes[3+i*4]==i*3 && bytes[4+i*4]==30+i*3 && bytes[5+i*4]==0 && bytes[6+i*4]==i+1);}assert(rewrite.countVertex(4)==3);for(unsigned i=15;i<rewrite.getDisplayListSize();++i)assert(bytes[i]==0);
        JKRHeap::free(const_cast<u8*>(bytes),&heap);assert(original[4]==0 && original[5]==1);
        std::printf("PASS original environment-map geometry rewrite: GX count byte order, inserted matrix selectors and unchanged indices\n");}
    {std::unique_ptr<NativeShapeResource> rewritable;for(auto block:modelBlocks.blocks){BinaryView section(modelBytes,block.offset,block.length);if(section.tag(0)=="SHP1")rewritable=std::make_unique<NativeShapeResource>(section);}
        J3DShape* selected=nullptr;unsigned stride=0;for(auto* shape:rewritable->pointers){unsigned size=0;bool positionMatrix=false;for(auto* descriptor=shape->getVtxDesc();descriptor->attr!=GX_VA_NULL;++descriptor){positionMatrix|=descriptor->attr==GX_VA_PNMTXIDX;const unsigned widths[4]={0,1,1,2};size+=widths[descriptor->type];}if(positionMatrix){selected=shape;stride=size;break;}}
        assert(selected && stride);auto* draw=selected->mShapeDraw[0];unsigned count=draw->countVertex(stride);assert(count);draw->addTexMtxIndexInDL(stride,1,30);assert(draw->countVertex(stride+1)==count);
        rewritable->retainRewrittenGeometry();rewritable->retainRewrittenGeometry();assert(rewritable->rewrittenLists.size()==1);u32 address=Native::gpuResources().byteAddress(draw->getDisplayList());assert(Native::gpuResources().resolve(address,draw->getDisplayListSize()));
        rewritable->prepareVertexCommands(retainedVertices->data);auto* oldCommands=selected->getVcdVatCmd();rewritable->makeCommandPrivate(*selected,heap);assert(selected->getVcdVatCmd()!=oldCommands);assert(std::memcmp(selected->getVcdVatCmd(),oldCommands,J3DShape::kVcdVatDLSize)==0);u32 privateAddress=Native::gpuResources().byteAddress(selected->getVcdVatCmd());selected->addTexMtxIndexInVcd(GX_VA_TEX0MTXIDX);rewritable->retainRewrittenDescriptors();rewritable->retainRewrittenDescriptors();assert(rewritable->rewrittenDescriptors.size()==1);bool inserted=false;for(auto* d=selected->getVtxDesc();d->attr!=GX_VA_NULL;++d)if(d->attr==GX_VA_TEX0MTXIDX){assert(d->type==GX_DIRECT);inserted=true;}assert(inserted);rewritable->prepareVertexCommands(retainedVertices->data);
        rewritable.reset();bool rejected=false;try{Native::gpuResources().resolve(address,1);}catch(const std::exception&){rejected=true;}assert(rejected);
        std::printf("PASS retained retail geometry rewrite: matrix selector insertion, idempotent registration and disposal\n");}
    {NativeOwnedModelData projected(modelBytes,heap);for(unsigned i=0;i<projected.getJointNum();++i)projected.getJointNodePointer(i)->setMtxCalc(&core);MR::invalidateMtxCalc(&projected);MR::invalidateJointCallback(&projected);for(unsigned i=0;i<projected.getJointNum();++i)assert(projected.getJointNodePointer(i)->getMtxCalc()==nullptr);bool expectedFur=false;for(unsigned i=0;i<projected.getMaterialNum();++i)expectedFur|=std::strstr(projected.getMaterialName()->getName(i),"Fur")!=nullptr;assert(MR::isUseFur(&projected)==expectedFur);auto& resource=projected.resources;
        J3DMaterial* chosen=nullptr;J3DTexMtx* matrix=nullptr;
        for(auto* material:resource.materialPointers){for(unsigned i=0;i<8;++i){auto* tex=material->mTexGenBlock->getTexMtx(i);if(tex){tex->getTexMtxInfo().mInfo=0;if(!chosen && material->getShape()->mHasPNMTXIdx){chosen=material;matrix=tex;}}}}
        assert(chosen && matrix);for(unsigned i=0;i<8;++i)chosen->getTexCoord(i)->mInfo.mTexGenSrc=4;chosen->getTexCoord(0)->mInfo.mTexGenSrc=1;matrix->getTexMtxInfo().mInfo=2;auto* shape=chosen->getShape();size_t before=resource.shapes->privateCommands.size();
        NativeModelInit::initEnvelopeAndEnvMapOrProjMapModelData(&projected,resource,heap);
        assert(resource.shapes->privateCommands.size()>before);assert(!resource.shapes->rewrittenLists.empty());assert(!resource.shapes->rewrittenDescriptors.empty());
        assert(Native::gpuResources().resolve(Native::gpuResources().byteAddress(shape->getVcdVatCmd()),J3DShape::kVcdVatDLSize));
        std::printf("PASS original projection-map initialization: retained commands, geometry and descriptors through native loader\n");}
    {auto* previous=JKRHeap::sCurrentHeap;JKRHeap::sCurrentHeap=nullptr;
        auto scoped=createNativeModelData(modelBytes.data(),modelBytes.size(),heap);assert(JKRHeap::sCurrentHeap==nullptr);
        auto malformed=modelBytes;for(auto block:modelBlocks.blocks)if(std::memcmp(malformed.data()+block.offset,"MAT3",4)==0)std::memcpy(malformed.data()+block.offset,"BAD!",4);
        bool rejected=false;try{createNativeModelData(malformed.data(),malformed.size(),heap);}catch(const std::exception&){rejected=true;}assert(rejected && JKRHeap::sCurrentHeap==nullptr);
        JKRHeap::sCurrentHeap=previous;scoped.reset();std::printf("PASS native loader heap scope: explicit heap and prior null restoration on success/failure\n");}
    {NativeOwnedModelData reduced(modelBytes,heap);auto& vertex=reduced.getVertexData();GXVtxAttrFmtList* position=nullptr;for(auto& format:reduced.resources.vertices->formats)if(format.attr==GX_VA_POS)position=&format;assert(position);
        position->type=GX_S16;position->cnt=GX_POS_XYZ;position->frac=3;vertex.setVtxPosType(GX_S16);vertex.setVtxPosFrac(3);vertex.mVtxNum=2;
        const int values[6]={-32768,-3,-1,0,3,32767};auto* bytes=static_cast<u8*>(vertex.getVtxPosArray());for(unsigned i=0;i<6;++i){u16 bits=u16(values[i]);bytes[i*2]=u8(bits>>8);bytes[i*2+1]=u8(bits);}
        reduced.downFracVtx();assert(position->frac==2 && vertex.getVtxPosFrac()==2);const int expected[6]={-16384,-2,-1,0,1,16383};auto snapshot=Native::gpuResources().resolve(Native::gpuResources().byteAddress(bytes),12);
        for(unsigned i=0;i<6;++i){u16 bits=u16(expected[i]);assert(bytes[i*2]==u8(bits>>8) && bytes[i*2+1]==u8(bits));assert(std::memcmp(snapshot+i*2,bytes+i*2,2)==0);}
        std::printf("PASS native fraction reduction: signed big-endian S16, metadata, GPU snapshot and regenerated shape commands\n");}
    {auto* previous=__GDCurrentDL;u8 bytes[32]{};GDLObj nested;GDInitGDLObj(&nested,bytes,sizeof(bytes));
        {Native::GDCurrentScope outer;GDSetCurrent(&nested);try{Native::GDCurrentScope inner;GDSetCurrent(nullptr);throw std::runtime_error("nested writer failure");}catch(const std::exception&){}assert(__GDCurrentDL==&nested);}
        assert(__GDCurrentDL==previous);std::printf("PASS native display-list writer scope: nested failure and caller restoration\n");}
    {J3DModelX::Flags flags{};u32 bits=0;flags._1F=1;std::memcpy(&bits,&flags,4);assert(bits==1);flags.clear();flags._10=1;std::memcpy(&bits,&flags,4);assert(bits==(1u<<15));flags.clear();flags._0=1;std::memcpy(&bits,&flags,4);assert(bits==(1u<<31));flags.clear();std::memcpy(&bits,&flags,4);assert(bits==0);std::printf("PASS specialized model flags: original PPC bit ordering on native host\n");}
    {for(unsigned i=0;i<8;++i)assert(__GDLightID2Index(GXLightID(1u<<i))==i);assert(__GDLightID2Index(GXLightID(0))==0);assert(__GDLightID2Index(GXLightID(0x80000000u))==0);assert(__GDLightID2Index(GXLightID(3))==1);std::printf("PASS original light ID conversion: eight channels, zero, multiple bits and out-of-range\n");}
    {u8 array[32]{},commands[32]{};u32 address=Native::gpuResources().add(array,sizeof(array));Native::GDCurrentScope writer;GDLObj output;GDInitGDLObj(&output,commands,sizeof(commands));GDSetCurrent(&output);GDSetArray(GX_VA_POS,array,12);assert(output.ptr-commands==12);assert(commands[0]==8 && commands[1]==0xa0 && commands[6]==8 && commands[7]==0xb0);u32 encoded=(u32(commands[2])<<24)|(u32(commands[3])<<16)|(u32(commands[4])<<8)|commands[5];assert(encoded==address && commands[11]==12);Native::gpuResources().remove(array);std::printf("PASS original geometry array writer: registered native address and stride CP commands\n");}
    {u8 image[32]{},commands[32]{};u32 address=Native::gpuResources().add(image,sizeof(image));Native::GDCurrentScope writer;GDLObj output;GDInitGDLObj(&output,commands,sizeof(commands));GDSetCurrent(&output);GDSetTexImgPtr(GX_TEXMAP0,image);assert(output.ptr-commands==5 && commands[0]==0x61 && commands[1]==0x94);u32 encoded=(u32(commands[2])<<16)|(u32(commands[3])<<8)|commands[4];assert(encoded==(address>>5));Native::gpuResources().remove(image);std::printf("PASS original texture image writer: registered native address in BP image command\n");}
    {u8 commands[64]{};Native::GDCurrentScope writer;GDLObj output;GDInitGDLObj(&output,commands,sizeof(commands));GDSetCurrent(&output);GDSetCurrentMtx(3,6,9,12,15,18,21,24,27);assert(output.ptr-commands==25);
        auto word=[&](unsigned i){return (u32(commands[i])<<24)|(u32(commands[i+1])<<16)|(u32(commands[i+2])<<8)|commands[i+3];};u32 a=3|(6u<<6)|(9u<<12)|(12u<<18)|(15u<<24);u32 b=18|(21u<<6)|(24u<<12)|(27u<<18);
        assert(commands[0]==8 && commands[1]==0x30 && word(2)==a);assert(commands[6]==8 && commands[7]==0x40 && word(8)==b);assert(commands[12]==0x10 && word(13)==0x00011018 && word(17)==a && word(21)==b);std::printf("PASS original transform writer: matrix selectors in CP and XF commands\n");}
    {alignas(16) u8 source[80],destination[80];for(unsigned i=0;i<80;++i)source[i]=u8(i*13);for(unsigned src=0;src<4;++src)for(unsigned dst=0;dst<4;++dst)for(unsigned size=0;size<=65;++size){std::memset(destination,0xa5,80);MR::copyMemory(destination+dst,source+src,size);assert(std::memcmp(destination+dst,source+src,size)==0);for(unsigned i=0;i<dst;++i)assert(destination[i]==0xa5);for(unsigned i=dst+size;i<80;++i)assert(destination[i]==0xa5);}std::printf("PASS original memory copy: alignment combinations, size boundaries and untouched guards\n");}
    {TVec3f value(2,3,4),normal(0,1,0),out;assert(MR::vecKillElement(value,normal,&out)==3 && out.x==2 && out.y==0 && out.z==4);assert(MR::vecKillElement(value,normal,&value)==3 && value.y==0);TVec3f zero(0,0,0);assert(MR::vecKillElement(value,zero,&out)==0 && out.x==value.x && out.z==value.z);std::printf("PASS native vector projection: original wrapper, axis removal, aliasing and zero direction\n");}
    {for(auto up:{TVec3f(0,1,0),TVec3f(0,0,1),TVec3f(2,3,4)}){TPos3f matrix;MR::makeMtxUpNoSupportPos(&matrix,up,TVec3f(7,8,9));auto* words=reinterpret_cast<const float*>(&matrix);assert(words[3]==7 && words[7]==8 && words[11]==9);for(unsigned column=0;column<3;++column){float length=0;for(unsigned row=0;row<3;++row)length+=words[row*4+column]*words[row*4+column];assert(std::fabs(length-1)<0.0001f);}for(unsigned a=0;a<3;++a)for(unsigned b=a+1;b<3;++b){float dot=0;for(unsigned row=0;row<3;++row)dot+=words[row*4+a]*words[row*4+b];assert(std::fabs(dot)<0.0001f);}}std::printf("PASS original orientation helpers: both support branches, orthonormal axes and translation\n");}
    {assert(GXGetTexBufferSize(608,448,GX_TF_RGB565,GX_FALSE,1)==608u*448u*2);assert(GXGetTexBufferSize(5,7,GX_TF_RGB565,GX_FALSE,1)==128);assert(GXGetTexBufferSize(5,7,GX_TF_RGBA8,GX_FALSE,1)==256);assert(GXGetTexBufferSize(5,7,GX_TF_CMPR,GX_FALSE,1)==32);assert(GXGetTexBufferSize(8,8,GX_TF_RGB565,GX_TRUE,4)==224);assert(GXGetTexBufferSize(8,8,GX_TF_RGBA8,GX_TRUE,4)==448);std::printf("PASS original texture sizing: screen RGB565, partial tiles and mip chains\n");}
    core.doFreeze();assert(core._28==1 && core._1C==0);
    J3DMtxCalc::setJoint(previousJoint);J3DMtxCalc::setMtxBuffer(previousBuffer);
    // Original destructor is empty; fixture owns the two constructor arrays.
    delete[] core.mTrackList;core.mTrackList=nullptr;delete[] core.mJointList;core.mJointList=nullptr;
    std::printf("PASS original XanimeCore run.bck: %zu joint updates, scale/translation references and rotation matrices\n",samples);
    std::printf("PASS original core: %zu pose transitions/cached fallbacks, track frame override and freeze request\n",transitions);
}








































































































































