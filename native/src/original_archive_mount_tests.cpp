#include <Game/Speaker/SpkData.hpp>
#include <Native/SpeakerResources.hpp>
#include <Native/CameraAnimationArchive.hpp>
#include <Native/ArchiveResource.hpp>
#include "assets.hpp"
#include "native_visibility_animation.hpp"
#include "native_texture_pattern_animation.hpp"
#include "animation.hpp"
#include <Native/BckAnimation.hpp>
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include "native_jkr_archive_port.hpp"
#include <Native/GpuResources.hpp>
#include "native_jkr_aram_port.hpp"
#include "native_dvd_port.hpp"
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <JSystem/JKernel/JKRFileFinder.hpp>
#include <JSystem/JKernel/JKRCompArchive.hpp>
#include <JSystem/JKernel/JKRDvdArchive.hpp>
#include <JSystem/JKernel/JKRAramArchive.hpp>
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <cassert>
#include <cstdio>
#include <Game/System/ErrorArchive.hpp>
int main(int argc,char** argv){
 assert(argc==2 || argc==3);const char* archivePath=argc==3?argv[2]:"/LayoutData/Font.arc";alignas(32) static unsigned char storage[8*1024*1024]{};
 JKRExpHeap parent(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&parent;JKRHeap::sSystemHeap=&parent;parent.becomeCurrentHeap();JKRFileLoader::initializeVolumeList();
 auto path=std::filesystem::u8path(argv[1]);WiiAssets disc(path);auto bytes=disc.readFile(archivePath);Rarc reference(bytes);
 size_t loadedAnimations=0,loadedCameras=0,loadedVisibility=0,loadedPatterns=0;
 struct RetainedPattern {std::unique_ptr<J3DAnmBase> owner;std::vector<u16> samples;std::vector<std::string> names;};
 std::vector<RetainedPattern> retainedPatterns;
 auto verify=[&](JKRArchive& archive){assert(archive.mIsMounted);bool rejected=false;int foreign=0;try{Native::copyArchiveResource(archive,&foreign);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);Native::CameraAnimationData camera;assert(!Native::decodeMountedCameraAnimation(camera,reinterpret_cast<const uint8_t*>(uintptr_t(1))));assert(!Native::decodeMountedCameraAnimation(camera,nullptr));assert(archive.countResource()==reference.files.size());for(const auto& file:reference.files){auto expected=reference.readFile(file.path);auto actual=archive.getResource(file.path.c_str());assert(actual && archive.getResSize(actual)==expected.size());assert(expected.empty() || !std::memcmp(actual,expected.data(),expected.size()));assert(Native::copyArchiveResource(archive,actual)==expected); if(expected.size()<8 || std::memcmp(expected.data(),"ANDO",4))assert(!Native::decodeMountedCameraAnimation(camera,static_cast<const uint8_t*>(actual)));
   if(expected.size()>=32 && !std::memcmp(expected.data(),"ANDO",4)){
     Native::CameraAnimationData referenceCamera;assert(referenceCamera.decode(expected.data(),expected.size()));
     assert(Native::decodeMountedCameraAnimation(camera,static_cast<const uint8_t*>(actual)));
     assert(camera.frames==referenceCamera.frames && camera.keyed==referenceCamera.keyed && camera.metadata==referenceCamera.metadata && camera.values==referenceCamera.values);
     ++loadedCameras;
   }
   if(expected.size()>=8 && !std::memcmp(expected.data(),"J3D1btp1",8)){
    std::unique_ptr<J3DAnmBase> loaded(loadNativeArchiveTexturePattern(archive,actual));assert(loaded&&loaded->getKind()==2);
    auto* pattern=static_cast<J3DAnmTexPattern*>(loaded.get());NativeTexturePatternAnimation expectedPattern(expected.data(),expected.size());
    assert(pattern->getUpdateMaterialNum()==expectedPattern.getUpdateMaterialNum());
    for(unsigned j=0;j<pattern->getUpdateMaterialNum();j++){
     assert(pattern->getUpdateMaterialID(j)==expectedPattern.getUpdateMaterialID(j));
     assert(!std::strcmp(pattern->mUpdateMaterialName.getName(j),expectedPattern.mUpdateMaterialName.getName(j)));
     for(float frame:{-2.f,0.f,.5f,1.5f,float(pattern->mFrameMax),1000.f}){
      pattern->mFrame=frame;expectedPattern.mFrame=frame;u16 a=0,b=0;pattern->getTexNo(j,&a);expectedPattern.getTexNo(j,&b);assert(a==b);
     }
    }
    int foreign=0;bool rejected=false;try{std::unique_ptr<J3DAnmBase> invalid(loadNativeArchiveTexturePattern(archive,&foreign));}
    catch(const std::exception&){rejected=true;}assert(rejected);assert(loadNativeArchiveTexturePattern(archive,nullptr)==nullptr);
    RetainedPattern retained;pattern->mFrame=1.5f;
    for(unsigned j=0;j<pattern->getUpdateMaterialNum();j++){u16 value=0;pattern->getTexNo(j,&value);retained.samples.push_back(value);retained.names.emplace_back(pattern->mUpdateMaterialName.getName(j));}
    retained.owner=std::move(loaded);retainedPatterns.push_back(std::move(retained));++loadedPatterns;
   }
   if(expected.size()>=8 && !std::memcmp(expected.data(),"J3D1bva1",8)){
    std::unique_ptr<J3DAnmBase> loaded(Native::loadArchiveVisibility(archive,actual));
    assert(loaded&&loaded->getKind()==6);
    auto* visibility=static_cast<J3DAnmVisibilityFull*>(loaded.get());
    auto referenceAnimation=Native::createVisibilityAnimation(expected.data(),expected.size());
    for(unsigned j=0;j<visibility->mUpdateMaterialNum;j++)for(float frame:{-2.f,0.f,.5f,float(visibility->mFrameMax),1000.f}){
     visibility->mFrame=frame;referenceAnimation->mFrame=frame;u8 a=9,b=9;
     visibility->getVisibility(j,&a);referenceAnimation->getVisibility(j,&b);assert(a==b);
    }
    int foreign=0;bool rejected=false;
    try{std::unique_ptr<J3DAnmBase> invalid(Native::loadArchiveVisibility(archive,&foreign));}
    catch(const std::invalid_argument&){rejected=true;}assert(rejected);
    assert(Native::loadArchiveVisibility(archive,nullptr)==nullptr);++loadedVisibility;
   }
   if(expected.size()>=8 && !std::memcmp(expected.data(),"J3D1bck1",8)){
     auto decoded=decodeBck(BinaryView(expected));
     std::unique_ptr<J3DAnmBase> loaded(Native::loadArchiveBck(archive,actual));assert(loaded && loaded->getKind()==8);
     auto* animation=static_cast<J3DAnmTransformKey*>(loaded.get());animation->mFrame=decoded.duration*0.5f;
     for(size_t joint=0;joint<decoded.joints.size();++joint){J3DTransformInfo transform;animation->getTransform(u16(joint),&transform);
       for(unsigned axis=0;axis<3;++axis){float value=decoded.joints[joint][axis*3+2].sample(animation->mFrame,0);
         assert(std::fabs((&transform.mTranslate.x)[axis]-value)<=0.0001f*(1+std::fabs(value)));
       }
     }
     ++loadedAnimations;
   }
 }
 if(std::strcmp(archivePath,"/AudioRes/SpkRes/SpkRes.arc")==0){
   auto tb=reference.readFile("/spktable.bct"), wb=reference.readFile("/spkwave.csw");
   Native::SpeakerResources expected;assert(expected.decode(tb.data(),tb.size(),wb.data(),wb.size()));
   auto* te=archive.findNameResource("spktable.bct");auto* we=archive.findNameResource("spkwave.csw");assert(te&&we);
   SpkData data(&archive);assert(!data.isValid());data.loadTable(te->mFileID);assert(!data.isValid());data.loadWave(we->mFileID);assert(data.isValid());
   auto* table=data.getSpkTable();auto* wave=data.getWave();assert(table->mResourceCount==expected.table().entries().size());
   for(size_t i=0;i<expected.table().entries().size();++i){
     const auto& e=expected.table().entries()[i];auto* a=table->getParameters(int(i));
     assert(a->mWaveID==e.wave&&a->_2==e.flags&&a->mVolume==e.volume&&a->mReleaseTime==e.release&&a->_padding==e.padding);
     assert(e.name==table->mNames[i]);
   }
   for(size_t i=0;i<expected.wave().waves().size();++i){
     const auto& e=expected.wave().waves()[i];assert(wave->getWaveSize(int(i))==e.samples.size()*2);
     assert(wave->getLoopStartPos(int(i))==e.loopStart&&wave->getLoopEndPos(int(i))==e.loopEnd);
     assert(std::equal(e.samples.begin(),e.samples.end(),wave->getWave(int(i))));
   }
 }
 };
 auto initial=parent.getTotalFreeSize();
 const void* detachedGpuResource=nullptr;
 {JKRMemArchive embedded;assert(embedded.mountFixed(const_cast<u8*>(cErrorArchive),JKR_MEM_BREAK_FLAG_0));assert(embedded.countResource()>0);auto* nested=embedded.getResource("/usenglish/layoutdata/embeddedfont.arc");assert(nested&&embedded.getResSize(nested)>0);detachedGpuResource=nested;const auto gpuAddress=Native::gpuResources().byteAddress(nested);assert(Native::gpuResources().resolve(gpuAddress,embedded.getResSize(nested))==nested);auto* finder=embedded.getFirstFile("/");assert(finder&&finder->mHasMoreFiles);delete finder;}
 bool gpuResourceDetached=false;try{Native::gpuResources().byteAddress(detachedGpuResource);}catch(const std::invalid_argument&){gpuResourceDetached=true;}assert(gpuResourceDetached);
 assert(parent.check() && parent.getTotalFreeSize()==initial);
 {assert(nativeRegisterRarc(bytes.data(),bytes.size()));JKRMemArchive archive;assert(archive.mountFixed(bytes.data(),JKR_MEM_BREAK_FLAG_0));nativeUnregisterRarc(bytes.data());bytes.clear();bytes.shrink_to_fit();verify(archive);}
 assert(parent.check() && parent.getTotalFreeSize()==initial);
 assert(nativeBindDvd(path));int entry=DVDConvertPathToEntrynum(archivePath);assert(entry>=0);
 {JKRCompArchive archive(entry,JKRArchive::MOUNT_DIRECTION_1);verify(archive);}
 assert(parent.check() && parent.getTotalFreeSize()==initial);
 {JKRDvdArchive archive(entry,JKRArchive::MOUNT_DIRECTION_1);verify(archive);} assert(parent.check() && parent.getTotalFreeSize()==initial); std::vector<uint8_t> backing(4*1024*1024);auto transport=std::make_shared<NativeAramMemory>(backing.data(),backing.size());assert(nativeBindAramTransport(transport));
 {JKRAram manager(0,0xffffffffu,16);JKRAram::sAramObject=&manager;{JKRAramArchive archive(entry,JKRArchive::MOUNT_DIRECTION_1);verify(archive);}}
 nativeReleaseAramTransport();nativeReleaseDvd();assert(parent.check() && parent.getTotalFreeSize()==initial);assert(JKRFileLoader::sVolumeList.getNumLinks()==0);
 for(auto& retained:retainedPatterns){
  auto* pattern=static_cast<J3DAnmTexPattern*>(retained.owner.get());pattern->mFrame=1.5f;
  for(unsigned j=0;j<pattern->getUpdateMaterialNum();j++){u16 value=0;pattern->getTexNo(j,&value);assert(value==retained.samples[j]);assert(retained.names[j]==pattern->mUpdateMaterialName.getName(j));}
 }
 retainedPatterns.clear();assert(parent.check()&&parent.getTotalFreeSize()==initial);
 std::printf("PASS original memory, combined, DVD and ARAM archive %s resource bytes and heap cleanup\n",archivePath);
 if(loadedPatterns)std::printf("PASS %zu BTP loads through mounted archive extent queries, material names/IDs, null and foreign-pointer checks\n",loadedPatterns);
 if(loadedVisibility)std::printf("PASS %zu BVA loads through original mounted archive size queries and foreign-pointer rejection\n",loadedVisibility);
 if(loadedCameras)std::printf("PASS %zu retail camera decodes through mounted archive ownership queries\n",loadedCameras);
 if(loadedAnimations)std::printf("PASS %zu BCK loads through original archive size queries and virtual object disposal\n",loadedAnimations);
}










