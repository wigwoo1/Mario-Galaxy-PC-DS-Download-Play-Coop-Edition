#include <Native/OwnedSpeakerWave.hpp>
#include <Native/OwnedSpeakerTable.hpp>
#include <Native/SpeakerResources.hpp>
#include <Native/SpeakerWaveData.hpp>
#include <Native/SpeakerTableData.hpp>
#include "assets.hpp"
#include <cstdio>
// Fixture-only original table storage initialization; resource loader is not executed.

SpkTable::SpkTable():mInitialized(false),mResourceCount(0),mParameters(nullptr),mNames(nullptr){}
int main(int argc,char** argv){try{
 assetCheck(argc==2,"Expected WBFS");WiiAssets disc(std::filesystem::u8path(argv[1]));
 auto data=disc.readFile("/AudioRes/SpkRes/SpkRes.arc");Rarc archive(data);
 for(const auto& f:archive.files){auto bytes=archive.readFile(f.path);
 std::printf("%s bytes=%zu header:",f.path.c_str(),bytes.size());
 for(size_t off=0;off+4<=bytes.size()&&off<32;off+=4)std::printf(" %08x",be32(bytes.data()+off));
 std::puts("");
 if(f.path=="/spktable.bct"){
  Native::SpeakerTableData table;assetCheck(table.decode(bytes.data(),bytes.size()),"Retail speaker table decode failed");
  assetCheck(table.entries().size()==34,"Unexpected retail speaker count");
  for(size_t i=0;i<table.entries().size();++i){
   const auto& entry=table.entries()[i];auto raw=bytes.data()+be32(bytes.data()+4)+i*8;
   assetCheck(entry.wave==uint16_t(raw[0]*256+raw[1])&&entry.volume==raw[3]&&entry.flags==raw[2],"Parameter mismatch");
   auto offset=be32(bytes.data()+be32(bytes.data()+8)+i*4);
   assetCheck(entry.name==reinterpret_cast<const char*>(bytes.data()+offset),"Name mismatch");
  }
  for(size_t size=0;size<16;++size)assetCheck(!table.decode(bytes.data(),size),"Truncated header accepted");
  auto bad=bytes;bad[12]=1;assetCheck(!table.decode(bad.data(),bad.size()),"Relocated table accepted");
  bad=bytes;for(int i=0;i<4;++i)bad[i]=255;
  assetCheck(!table.decode(bad.data(),bad.size()),"Oversized count accepted");
  bad=bytes;auto names=be32(bytes.data()+8);for(int i=0;i<4;++i)bad[names+i]=255;
  assetCheck(!table.decode(bad.data(),bad.size()),"Invalid name offset accepted");
  assetCheck(table.entries().size()==34,"Failure changed prior table");
  bytes.clear();bytes.shrink_to_fit();
  assetCheck(!table.entries()[0].name.empty(),"Owned names lost");
  std::puts("PASS retail34 speaker entries/endian parameters/owned names/truncation/count/name offset/transactional failures");
 }
 if(f.path=="/spkwave.csw"){
  Native::SpeakerWaveData decoded;assetCheck(decoded.decode(bytes.data(),bytes.size()),"Wave decode failed");
  assetCheck(decoded.waves().size()==34,"Wave count mismatch");
  for(size_t i=0;i<34;++i){
   size_t off=be32(bytes.data()+8+i*4);const auto& wave=decoded.waves()[i];
   assetCheck(wave.samples.size()*2==be32(bytes.data()+off),"Sample count mismatch");
   for(size_t j=0;j<wave.samples.size();++j){
    uint16_t raw=uint16_t(bytes[off+12+j*2])*256+bytes[off+13+j*2];
    int32_t expected=raw<32768?raw:int32_t(raw)-65536;
    assetCheck(wave.samples[j]==expected,"Sample endian mismatch");
   }
  }
  auto bad=bytes;bad[0]=255;assetCheck(!decoded.decode(bad.data(),bad.size()),"Bad total accepted");
  bad=bytes;size_t first=be32(bytes.data()+8);bad[first+3]|=1;
  assetCheck(!decoded.decode(bad.data(),bad.size()),"Odd PCM bytes accepted");
  assetCheck(!decoded.decode(bytes.data(),bytes.size()-1),"Truncation accepted");
  assetCheck(decoded.waves().size()==34,"Failure changed wave storage");
  assetCheck(decoded.waves()[7].loopStart==10&&decoded.waves()[7].loopEnd==40,"Retail loop mismatch");
  std::puts("PASS retail34 waves/all signed PCM samples/loop metadata/size and truncation failures");
  for(size_t i=0;i<be32(bytes.data()+4);++i){
   size_t off=be32(bytes.data()+8+i*4);extent(off,12,bytes.size());
   std::printf("wave%zu offset=%zu bytes=%u loop=%u..%u",i,off,be32(bytes.data()+off),be32(bytes.data()+off+4),be32(bytes.data()+off+8));std::puts("");
  }
 }
 }
 auto tableBytes=archive.readFile("/spktable.bct"),waveBytes=archive.readFile("/spkwave.csw");
 Native::SpeakerResources resources;
 assetCheck(resources.decode(tableBytes.data(),tableBytes.size(),waveBytes.data(),waveBytes.size()),"Combined decode failed");
 auto badTable=tableBytes;size_t param=be32(tableBytes.data()+4);badTable[param]=255;badTable[param+1]=255;
 assetCheck(!resources.decode(badTable.data(),badTable.size(),waveBytes.data(),waveBytes.size()),"Invalid wave reference accepted");
 auto writeWord=[](std::vector<uint8_t>& b,size_t off,uint32_t value){for(int i=0;i<4;++i)b[off+i]=uint8_t(value>>(24-8*i));};
 size_t loop=be32(waveBytes.data()+8+7*4);
 auto badWave=waveBytes;writeWord(badWave,loop+4,40);writeWord(badWave,loop+8,10);
 assetCheck(!resources.decode(tableBytes.data(),tableBytes.size(),badWave.data(),badWave.size()),"Reversed loop accepted");
 badWave=waveBytes;writeWord(badWave,loop+8,41);
 assetCheck(!resources.decode(tableBytes.data(),tableBytes.size(),badWave.data(),badWave.size()),"Loop outside samples accepted");
 badWave=waveBytes;writeWord(badWave,loop+4,UINT32_MAX);
 assetCheck(!resources.decode(tableBytes.data(),tableBytes.size(),badWave.data(),badWave.size()),"Mixed loop sentinel accepted");
 assetCheck(resources.table().entries().size()==34&&resources.wave().waves()[7].loopStart==10,"Failure changed combined resources");
 Native::OwnedSpeakerTable ownedTable;SpkTable originalTable;
 assetCheck(ownedTable.load(tableBytes.data(),tableBytes.size()),"Owned table load failed");ownedTable.bind(originalTable);
 assetCheck(originalTable.mInitialized&&originalTable.mResourceCount==34,"Original table binding failed");
 for(int i=0;i<34;++i){const auto& expected=resources.table().entries()[i];auto actual=originalTable.getParameters(i);
  assetCheck(actual->mWaveID==expected.wave&&actual->_2==expected.flags&&actual->mVolume==expected.volume&&actual->mReleaseTime==expected.release&&actual->_padding==expected.padding,"Original table parameter mismatch");
  assetCheck(expected.name==originalTable.mNames[i],"Original table name mismatch");
 }
 auto retainedNames=originalTable.mNames;assetCheck(!ownedTable.load(badTable.data(),badTable.size()-badTable.size()),"Empty table accepted");
 assetCheck(originalTable.mNames==retainedNames&&originalTable.mResourceCount==34,"Failed load invalidated original table");
 std::puts("PASS owned native table binding: original34 parameter getters/name pointers/failed reload preservation");
 Native::OwnedSpeakerWave ownedWave;SpkWave originalWave;
 assetCheck(ownedWave.load(waveBytes.data(),waveBytes.size()),"Owned wave load failed");ownedWave.bind(originalWave);
 assetCheck(originalWave.mResource!=nullptr&&ownedWave.view(34)==nullptr,"Wave binding/range mismatch");
 for(size_t i=0;i<34;++i){
  const auto& expected=resources.wave().waves()[i];auto view=originalWave.getWaveData(static_cast<s32>(i));
  assetCheck(originalWave.getWaveSize(i)==expected.samples.size()*2&&originalWave.getLoopStartPos(i)==expected.loopStart&&originalWave.getLoopEndPos(i)==expected.loopEnd&&originalWave.getWave(i)==view->mWave&&view->mSize==expected.samples.size()*2&&view->mLoopStartPos==expected.loopStart&&view->mLoopEndPos==expected.loopEnd,"Native wave header mismatch");
  for(size_t j=0;j<expected.samples.size();++j)assetCheck(view->mWave[j]==expected.samples[j],"Bound PCM mismatch");
 }
 auto retainedWave=originalWave.mResource;assetCheck(!ownedWave.load(nullptr,0),"Empty wave accepted");
 assetCheck(originalWave.mResource==retainedWave&&ownedWave.view(7)->mLoopEndPos==40,"Failed load changed bound wave");
 std::puts("PASS owned wave binding: original host layout/all34 PCM buffers/loop metadata/failed reload");
 tableBytes.clear();waveBytes.clear();
 assetCheck(!resources.table().entries()[0].name.empty()&&resources.wave().waves()[0].samples.size()==4048,"Combined ownership lost");
 std::puts("PASS combined retail speaker resources: wave references/loop ranges/mixed sentinel/atomic failures/source lifetime");
 return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}









