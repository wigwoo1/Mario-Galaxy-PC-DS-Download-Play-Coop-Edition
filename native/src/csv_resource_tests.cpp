#include "native_disc_archive_loader.hpp"
#include "stage_data.hpp"
#include <Game/Util/JMapInfo.hpp>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <cstring>
#include <cassert>

// Only archive lookup/metadata are fixtures. The CSV factory helper and
// JMapInfo implementation are extracted/compiled from production source.
struct ResFileInfo {void* mResource;u32 _4;};
struct ResTable {
    ResFileInfo info;
    bool isExistRes(const char* name)const{return std::strcmp(name,"PlanetMapDataTable.bcsv")==0;}
    void* getRes(const char* name)const{return isExistRes(name)?info.mResource:nullptr;}
    const ResFileInfo* findFileInfo(const char* name)const{return isExistRes(name)?&info:nullptr;}
};
struct ResourceHolder {ResTable* mFileInfoTable;};
#ifndef NO_INLINE
#define NO_INLINE
#endif
// ACTUAL_CSV_FACTORY_INSERTION_POINT

static std::unique_ptr<JMapInfo> parse(const ResourceHolder* holder,const char* format,...){
    va_list args;va_start(args,format);
    try{auto* result=tryCreateCsvParserLocal(holder,format,args);va_end(args);return std::unique_ptr<JMapInfo>(result);}
    catch(...){va_end(args);throw;}
}
int main(int argc,char** argv){
    assert(argc==2);
    NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));
    const char* archiveName="/ObjectData/PlanetMapDataTable.arc";
    auto archive=loader.mount(archiveName);
    const uint8_t* resource=nullptr;
    for(const auto& file:archive->files()){
        auto name=file.path.substr(file.path.find_last_of('/')+1);
        for(auto& c:name)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
        if(name=="planetmapdatatable.bcsv"){resource=archive->get(file.path);break;}
    }
    assert(resource);
    const auto size=archive->sizeOf(resource);
    std::vector<uint8_t> before(resource,resource+size);
    StageTable reference{BinaryView(before)};reference.validate();
    ResTable files{{const_cast<uint8_t*>(resource),u32(size)}};
    ResourceHolder holder{&files};
    auto table=parse(&holder,"%s.%s","PlanetMapDataTable","bcsv");
    assert(table);
    std::printf("Retail planet CSV: %zu bytes, %zu rows, %zu fields; parser reports %d rows, %d fields\n",size,reference.rows,reference.fields.size(),table->getNumEntries(),table->getNumFields());
    if(size_t(table->getNumEntries())!=reference.rows||size_t(table->getNumFields())!=reference.fields.size()){
        std::puts("FAIL raw CSV attachment exposes disk-endian header values");return 10;
    }
    assert(std::memcmp(resource,before.data(),size)==0);
    assert(!parse(&holder,"missing.bcsv"));
    files.info._4=15;bool rejected=false;
    try{parse(&holder,"PlanetMapDataTable.bcsv");}catch(const std::exception&){rejected=true;}
    assert(rejected);files.info._4=u32(size);
    std::vector<uint8_t> malformed=before;malformed[4]=0x7f;
    files.info.mResource=malformed.data();rejected=false;
    try{parse(&holder,"PlanetMapDataTable.bcsv");}catch(const std::exception&){rejected=true;}
    assert(rejected);
    assert(loader.unmount(archiveName));archive.reset();
    for(size_t row=0;row<reference.rows;++row)for(size_t field=0;field<reference.fields.size();++field){
        const auto expected=reference.read(row,field);const auto type=reference.fields[field].type;
        if(type==1||type==6){const char* value=nullptr;assert(table->getValueFast(int(row),int(field),&value));assert(value&&std::get<std::string>(expected)==value);}
        else if(type==2){float value;assert(table->getValueFast(int(row),int(field),&value));const float wanted=std::get<float>(expected);assert(std::memcmp(&value,&wanted,4)==0);}
        else{u32 value;assert(table->getValueFast(int(row),int(field),&value));assert(value==std::get<uint32_t>(expected));}
    }
    std::puts("PASS actual CSV factory: retail planet cells match independent reader, archive bytes stay unchanged, owned table survives archive release, malformed/truncated resources reject, missing resources stay null");
}
