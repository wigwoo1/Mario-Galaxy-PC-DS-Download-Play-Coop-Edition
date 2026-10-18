#include "native_disc_archive_loader.hpp"
#include "stage_data.hpp"
#include <Game/Camera/DotCamParams.hpp>
#include <cassert>
#include <cstdio>
int main(int argc,char** argv){
 assert(argc==2);NativeDiscArchiveLoader disc(std::filesystem::u8path(argv[1]));auto archive=disc.mount("/StageData/FileSelect.arc");
 const uint8_t* resource=nullptr;for(const auto& file:archive->files())if(file.path.find("cameraparam.bcam")!=std::string::npos)resource=archive->get(file.path);
 assert(resource);const auto size=archive->sizeOf(resource);std::vector<uint8_t> original(resource,resource+size);
 StageTable reference{BinaryView(original)};reference.validate();DotCamReaderInBin reader(resource,s32(size));
 assert(size_t(reader.mMapInfo.getNumEntries())==reference.rows);size_t rows=0,cells=0,vectors=0;
 const int version=reference.find("version");assert(version>=0&&reader.getVersion()==std::get<uint32_t>(reference.read(0,version)));
 while(reader.hasMoreChunk()){
  for(size_t field=0;field<reference.fields.size();++field){auto expected=reference.read(rows,field);const auto type=reference.fields[field].type;
   if(type==1||type==6){const char* value=nullptr;assert(reader.mMapInfo.getValueFast(int(rows),int(field),&value));assert(value&&std::get<std::string>(expected)==value);}
   else if(type==2){float value;assert(reader.mMapInfo.getValueFast(int(rows),int(field),&value));float wanted=std::get<float>(expected);assert(std::memcmp(&value,&wanted,4)==0);}
   else{u32 value;assert(reader.mMapInfo.getValueFast(int(rows),int(field),&value));assert(value==std::get<uint32_t>(expected));}++cells;
  }
  const int id=reference.find("id");if(id>=0){const char* value=nullptr;assert(reader.getValueString("id",&value));assert(std::get<std::string>(reference.read(rows,id))==value);}
  for(const char* name:{"woffset","vpanaxis","axis","wpoint","up"}){
   TVec3f value;if(reader.getValueVec(name,&value)){float actual[]={value.x,value.y,value.z};const char* suffix[]={".X",".Y",".Z"};
    for(unsigned i=0;i<3;++i){int column=reference.find(std::string(name)+suffix[i]);assert(column>=0&&actual[i]==std::get<float>(reference.read(rows,column)));}++vectors;}
  }
  ++rows;reader.nextToChunk();
 }
 assert(rows==reference.rows&&rows>0&&vectors>0);reader.nextToChunk();assert(!reader.hasMoreChunk());
 assert(std::memcmp(resource,original.data(),size)==0);assert(disc.unmount("/StageData/FileSelect.arc"));archive.reset();
 reader.mMapIter=reader.mMapInfo.begin();const char* retained=nullptr;assert(reader.getValueString("id",&retained)&&retained);
 const auto entriesEnd=uint64_t(be32(original.data()+8))+uint64_t(be32(original.data()))*be32(original.data()+12);
 for(s32 truncated:{0,15,s32(entriesEnd-1),-1}){bool rejected=false;try{DotCamReaderInBin invalid(original.data(),truncated);}catch(const std::exception&){rejected=true;}assert(rejected);}
 bool rejected=false;try{DotCamReaderInBin invalid(nullptr,s32(size));}catch(const std::exception&){rejected=true;}assert(rejected);
 std::printf("PASS actual DotCam reader: %zu real FileSelect rows, %zu cells, %zu vectors, immutable bytes, archive-release lifetime and truncated input rejection\n",rows,cells,vectors);
}
