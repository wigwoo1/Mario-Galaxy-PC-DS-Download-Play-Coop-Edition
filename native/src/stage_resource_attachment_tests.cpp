#include "native_disc_archive_loader.hpp"
#include "stage_data.hpp"
#include <Game/Util/JMapInfo.hpp>
#include <cassert>
#include <cstdio>
#include <Native/StageArchiveTables.hpp>

// Isolate the production ownership method from unrelated game services.
class StageDataHolder {
public:
    MR::AssignableArray<JMapInfo> mPlacementObjs, mStartObjs, mGeneralPosObjs,
        mChildObjs, mPathObjs, _EC, _F4;
    s32 mStageDataHolderCount=0;
    StageDataHolder* mStageDataArray[24]{};
    const StageDataHolder* findPlacedStageDataHolder(const JMapInfoIter&) const;
};
// ACTUAL_STAGE_OWNERSHIP_INSERTION_POINT

static void checkOwnership(const JMapInfo& general,const JMapInfo& queued){
    if(!queued.getNumEntries())return;
    assert(general.mData!=queued.mData);
    StageDataHolder root,child;
    root.mStageDataHolderCount=1;root.mStageDataArray[0]=&child;
    child.mPlacementObjs.init(1);child.mPlacementObjs[0]=general;
    assert(root.findPlacedStageDataHolder(JMapInfoIter(&general,0))==&child);
    // The common and scenario queues decode the same archive independently.
    child._EC.init(1);child._EC[0]=queued;
    assert(root.findPlacedStageDataHolder(JMapInfoIter(&queued,0))==&child);
    child._EC[0]=JMapInfo();child._F4.init(1);child._F4[0]=queued;
    assert(root.findPlacedStageDataHolder(JMapInfoIter(&queued,queued.getNumEntries()-1))==&child);
    assert(!root.findPlacedStageDataHolder(JMapInfoIter(&queued,-1)));
    assert(!root.findPlacedStageDataHolder(JMapInfoIter(&queued,queued.getNumEntries())));
    assert(!root.findPlacedStageDataHolder(JMapInfoIter()));
    JMapInfo unrelated;assert(!root.findPlacedStageDataHolder(JMapInfoIter(&unrelated,0)));
    child._F4[0]=JMapInfo();
    assert(!root.findPlacedStageDataHolder(JMapInfoIter(&queued,0)));
    assert(root.findPlacedStageDataHolder(JMapInfoIter(&general,0))==&child);
}

// The metadata boundary supplies real archive extents. Parsing and ownership
// use the actual production attachment helper and JMapInfo implementation.
class JKRArchive {
public:
    const OwnedArchiveResources* data=nullptr;
    s32 extentOverride=-2;
    s32 getResSize(const void* resource)const{
        if(extentOverride!=-2)return extentOverride;
        const auto size=data->sizeOf(resource);assert(size<=0x7fffffff);return s32(size);
    }
};
// ACTUAL_CSV_FACTORY_INSERTION_POINT

static void compare(const JMapInfo& info,const std::vector<uint8_t>& bytes){
    StageTable reference{BinaryView(bytes)};reference.validate();
    assert(size_t(info.getNumEntries())==reference.rows&&size_t(info.getNumFields())==reference.fields.size());
    for(size_t row=0;row<reference.rows;++row)for(size_t field=0;field<reference.fields.size();++field){
        const auto expected=reference.read(row,field);const auto type=reference.fields[field].type;
        if(type==1||type==6){const char* value=nullptr;assert(info.getValueFast(int(row),int(field),&value));assert(value&&std::get<std::string>(expected)==value);}
        else if(type==2){float value;assert(info.getValueFast(int(row),int(field),&value));const float wanted=std::get<float>(expected);assert(std::memcmp(&value,&wanted,4)==0);}
        else{u32 value;assert(info.getValueFast(int(row),int(field),&value));assert(value==std::get<uint32_t>(expected));}
    }
}
int main(int argc,char** argv){
    assert(argc==2);NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));
    size_t tables=0,rows=0;
    for(const char* name:{"/StageData/FileSelect.arc","/StageData/ObjNameTable.arc"}){
        auto archive=loader.mount(name);JKRArchive metadata;metadata.data=archive.get();
        std::vector<std::pair<JMapInfo,std::vector<uint8_t>>> retained;
        for(const auto& file:archive->files()){
            auto path=file.path;for(auto& c:path)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
            if(path.rfind("/jmp/",0)!=0&&path.find("objnametable.tbl")==std::string::npos)continue;
            const auto* resource=archive->get(file.path);const auto size=archive->sizeOf(resource);
            std::vector<uint8_t> before(resource,resource+size);JMapInfo info;
            attachStageArchiveTable(info,&metadata,resource);compare(info,before);
            JMapInfo queued;attachStageArchiveTable(queued,&metadata,resource);
            checkOwnership(info,queued);
            assert(std::memcmp(resource,before.data(),size)==0);
            JMapInfo rejectedTable;metadata.extentOverride=15;bool rejected=false;
            try{attachStageArchiveTable(rejectedTable,&metadata,resource);}catch(const std::exception&){rejected=true;}
            assert(rejected&&!rejectedTable.dataExists());metadata.extentOverride=-2;
            rows+=size_t(info.getNumEntries());++tables;
            retained.emplace_back(info,std::move(before));
        }
        if(retained.empty()){
            std::printf("No matching tables in %s; archive paths:\n",name);
            for(const auto& file:archive->files())std::puts(file.path.c_str());
            return 2;
        }
        assert(loader.unmount(name));archive.reset();
        for(const auto& table:retained)compare(table.first,table.second);
    }
    std::printf("PASS actual stage attachment: %zu real FileSelect/object-name tables, %zu rows match independent reader; immutable bytes, bounded extents and retained ownership\n",tables,rows);
    std::puts("PASS actual zone lookup: common/scenario queues, child-zone recursion, row bounds and unowned-table rejection");
}
