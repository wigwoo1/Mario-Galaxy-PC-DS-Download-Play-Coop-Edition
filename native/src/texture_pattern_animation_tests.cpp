#include "native_texture_pattern_animation.hpp"
#include "assets.hpp"
#include "native_model_blocks.hpp"
#include <JSystem/J3DGraphAnimator/J3DMaterialAttach.hpp>
#include <JSystem/J3DGraphAnimator/J3DMaterialAnm.hpp>
#include <cassert>
#include <cstdio>
int main(int argc,char** argv){
 assert(argc==2);WiiAssets disc(std::filesystem::u8path(argv[1]));size_t files=0,samples=0;
 for(const auto& entry:disc.files){
  if(entry.path.find("/ObjectData/")!=0||entry.path.size()<4||entry.path.substr(entry.path.size()-4)!=".arc"||entry.size>4*1024*1024)continue;
  Rarc archive(disc.readFile(entry.path));
  for(const auto& f:archive.files){
   auto source=archive.readFile(f.path);if(source.size()<8||std::memcmp(source.data(),"J3D1btp1",8))continue;
   auto raw=source;auto a=std::make_unique<NativeTexturePatternAnimation>(source.data(),source.size());source.clear();source.shrink_to_fit();
   BinaryView view(raw);size_t at=32;while(std::memcmp(raw.data()+at,"TPT1",4))at+=view.u32(at+4);auto block=view.sub(at,view.u32(at+4));
   for(unsigned j=0;j<a->mUpdateMaterialNum;j++){
    size_t t=block.u32(16)+j*8;int count=block.u16(t);unsigned offset=block.u16(t+2);
    assert(a->getUpdateMaterialID(j)==block.u16(block.u32(24)+j*2));
    assert(a->mAnmTable[j].mTexNo==block.u8(t+4));
    size_t nt=block.u32(28);size_t nameOffset=block.u16(nt+6+j*4);assert(!std::strcmp(a->mUpdateMaterialName.getName(j),reinterpret_cast<const char*>(raw.data()+at+nt+nameOffset)));
    for(float frame:{-2.f,0.f,.5f,1.5f,float(count),float(count+10)}){
     a->mFrame=frame;u16 tex=0;a->getTexNo(j,&tex);J3DTexNoAnm original(j,a.get()),retained;retained=original;u16 attached=0;retained.calc(&attached);assert(attached==tex&&retained.getAnmFlag()&&retained.getAnmTexPattern()==a.get());int index=std::max(0,std::min(count-1,int(frame)));
     assert(tex==block.u16(block.u32(20)+(offset+index)*2));++samples;
    }
   }
   bool matchedModel=false;
   for(const auto& modelFile:archive.files){
    auto model=archive.readFile(modelFile.path);
    if(model.size()<8||std::memcmp(model.data(),"J3D2",4)||(std::memcmp(model.data()+4,"bdl4",4)&&std::memcmp(model.data()+4,"bmd3",4)))continue;
    NativeModelBlocks blocks(model.data(),model.size());
    for(const auto& section:blocks.blocks){
     if(std::memcmp(model.data()+section.offset,"MAT3",4))continue;
     BinaryView mv(model);auto mat=mv.sub(section.offset,section.length);size_t nt=mat.u32(20);
     NativeNameTable modelNames(mat.sub(nt,mat.size()-nt));JUTNameTab names(modelNames.resource());
     J3DMaterialTable materials;materials.mMaterialName=&names;materials.mMaterialNum=names.mNameNum;
     std::vector<int> expectedIDs;for(unsigned j=0;j<a->getUpdateMaterialNum();j++)expectedIDs.push_back(names.getIndex(a->mUpdateMaterialName.getName(j)));
     assert(std::any_of(expectedIDs.begin(),expectedIDs.end(),[](int id){return id>=0;}));
     a->searchUpdateMaterialID(&materials);
     for(unsigned j=0;j<a->getUpdateMaterialNum();j++){assert(a->getUpdateMaterialID(j)==u16(expectedIDs[j]));assert(a->isValidUpdateMaterialID(j)==(expectedIDs[j]>=0));}
     names.setResource(nullptr);a->searchUpdateMaterialID(&materials);
     for(unsigned j=0;j<a->getUpdateMaterialNum();j++)assert(a->getUpdateMaterialID(j)==0xffff&&!a->isValidUpdateMaterialID(j));
     names.setResource(modelNames.resource());a->searchUpdateMaterialID(&materials);
     for(unsigned j=0;j<a->getUpdateMaterialNum();j++)assert(a->getUpdateMaterialID(j)==u16(expectedIDs[j]));
     matchedModel=true;
    }
   }
   assert(matchedModel);
   unsigned rejected=0;
   for(int kind=0;kind<5;kind++){
    auto corrupt=raw;
    if(kind==0)corrupt.resize(31);
    if(kind==1){corrupt[at+16]=0xff;corrupt[at+17]=0xff;}
    if(kind==2){size_t t=at+block.u32(16);corrupt[t]=0;corrupt[t+1]=0;}
    if(kind==3){size_t name=at+block.u32(28);corrupt[name]=0xff;corrupt[name+1]=0xff;}
    if(kind==4)corrupt[at+block.u32(16)+4]=8;
    try{NativeTexturePatternAnimation bad(corrupt.data(),corrupt.size());}
    catch(const std::exception&){++rejected;}
   }
   assert(rejected==5);++files;
  }
  if(files){std::printf("Retail BTP archive: %s\n",entry.path.c_str());break;}
 }
 assert(files>0);std::printf("PASS %zu retail BTP files, %zu original texture samples, material IDs/names, real MAT3 matching, missing-name sentinel/rematch, animator track-copy sampling, slot bounds and source disposal\n",files,samples);
}





