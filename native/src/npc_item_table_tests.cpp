#define main originalHeapTestEntry
#include "unit_heap_live_tests.cpp"
#undef main
#include <Native/NpcItemTable.hpp>
#include <Game/NPC/NPCParameter.hpp>
#include "native_disc_archive_loader.hpp"
#include "native_rarc_tables.hpp"
#include "stage_data.hpp"
#include <string>
#include <utility>

// ORIGINAL_NPC_ITEM_READERS

struct NpcTableFixture {
    ResFileInfo resource;
    std::vector<std::uint8_t> disk;
    std::string name;
};
int main(int argc,char** argv) {
    assert(argc==2);
    NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));
    auto archive=loader.mount("/ObjectData/NPCData.arc");
    WiiAssets disc(std::filesystem::u8path(argv[1]));
    NativeRarcTables archiveMetadata(disc.readFile("/ObjectData/NPCData.arc"));
    const char* archiveName=reinterpret_cast<const char*>(archiveMetadata.bytes.data()+archiveMetadata.strings+archiveMetadata.directories.front().mNameOffset);
    std::vector<NpcTableFixture> tables;tables.reserve(archive->files().size());
    std::vector<std::pair<const char*,std::string>> retained;
    alignas(32) u8 storage[262144]{};
    JKRExpHeap root(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&root;root.becomeCurrentHeap();
    const auto rootFree=root.getTotalFreeSize();
    auto* scene=JKRExpHeap::create(131072,&root,false);assert(scene);
    NPCItemParameterReader reader("NPC regression");
    const char* keys[]={"mGoods0","mGoods1","mGoodsJoint0","mGoodsJoint1"};
    std::size_t rows=0,rawHeaderMismatches=0;
    for(const auto& file:archive->files()) {
        std::string lower=file.path;for(char& c:lower)if(c>='A'&&c<='Z')c+=32;
        if(lower.size()<9||lower.substr(lower.size()-9)!="item.bcsv")continue;
        tables.emplace_back();auto& fixture=tables.back();
        const auto* raw=archive->get(file.path);const auto bytes=archive->sizeOf(raw);
        fixture.disk.assign(raw,raw+bytes);StageTable reference{BinaryView(fixture.disk)};reference.validate();
        fixture.name=file.path;
        auto& info=fixture.resource;info.mResource=info._8=const_cast<std::uint8_t*>(raw);info._4=static_cast<u32>(bytes);info.mName=fixture.name.data();
        {JMapInfo original;original.attach(raw);if(original.getNumFields()!=int(reference.fields.size()))++rawHeaderMismatches;}
        assert(!Native::prepareNpcItemTable("OtherArchive",info,scene)&&info.mResource==info._8);
        assert(Native::prepareNpcItemTable(archiveName,info,scene)&&info.mResource!=info._8);
        assert(std::memcmp(raw,fixture.disk.data(),bytes)==0);
    }
    assert(!tables.empty());
    const auto freeBefore=scene->getTotalFreeSize();const auto ownersBefore=scene->mDisposerList.getNumLinks();
    ResFileInfo invalid;invalid.mResource=invalid._8=tables.front().disk.data();invalid._4=15;
    bool rejected=false;try{Native::npcItemTable(invalid,scene);}catch(const std::invalid_argument&){rejected=true;}assert(rejected&&invalid.mResource==invalid._8);
    auto malformed=tables.front().disk;malformed[4]=0x7f;
    invalid.mResource=invalid._8=malformed.data();invalid._4=u32(malformed.size());
    rejected=false;try{Native::npcItemTable(invalid,scene);}catch(const std::exception&){rejected=true;}
    assert(rejected&&invalid.mResource==invalid._8&&scene->getTotalFreeSize()==freeBefore&&scene->mDisposerList.getNumLinks()==ownersBefore&&scene->check());
    assert(scene->adjustSize()>0&&scene->getMaxAllocatableSize(0)<sizeof(Native::NpcItemTableOwner));
    // Original stationed heaps have no room for a late owner allocation after
    // trimming. All real reads must reuse the archive-registration preparation.
    for(auto& fixture:tables) {
        auto& info=fixture.resource;StageTable reference{BinaryView(fixture.disk)};
        for(std::size_t row=0;row<reference.rows;++row) {
            NPCActorItem defaults("NPC regression");reader.copy(&defaults);
            {
                JMapInfo borrowed;borrowed.attach(Native::npcItemTable(info,scene));
                assert(borrowed.getNumEntries()==int(reference.rows)&&borrowed.getNumFields()==int(reference.fields.size()));
                const auto owners=scene->mDisposerList.getNumLinks();const auto* cached=info.mResource;
                assert(Native::npcItemTable(info,scene)==cached&&scene->mDisposerList.getNumLinks()==owners);
                reader.read(&borrowed,int(row));
                reader.read(&borrowed,-1);reader.read(&borrowed,int(reference.rows));
            }
            // The original reader returns borrowed pointers; the local table is gone.
            const char* values[]={reader.mItem.mGoods0,reader.mItem.mGoods1,reader.mItem.mGoodsJoint0,reader.mItem.mGoodsJoint1};
            for(unsigned field=0;field<4;++field) {
                const int index=reference.find(keys[field]);
                const auto expected=index<0?std::string():std::get<std::string>(reference.read(row,index));
                assert(values[field]&&expected==values[field]);retained.emplace_back(values[field],expected);
            }
            ++rows;
        }
    }
    assert(!tables.empty()&&rows>0&&rawHeaderMismatches>0&&!retained.empty());
    assert(loader.unmount("/ObjectData/NPCData.arc"));archive.reset();
    for(const auto& value:retained)assert(value.second==value.first);
    scene->freeAll();assert(scene->mDisposerList.getNumLinks()==0&&scene->check());
    for(const auto& fixture:tables)assert(fixture.resource.mResource==fixture.resource._8);
    // A second decode after bulk disposal must be possible without a stale cache.
    auto& first=tables.front().resource;first.mResource=first._8=tables.front().disk.data();
    Native::npcItemTable(first,scene);assert(first.mResource!=first._8);
    scene->do_destroy();assert(first.mResource==first._8&&root.getTotalFreeSize()==rootFree&&root.check());
    delete static_cast<NPCParameterEdit*>(reader[0]);delete static_cast<NPCParameterEdit*>(reader[1]);
    delete static_cast<NPCParameterJoint*>(reader[2]);delete static_cast<NPCParameterJoint*>(reader[3]);
    std::printf("PASS original NPC item readers: %zu retail tables, %zu rows, %zu string values; raw headers rejected by comparison\n",tables.size(),rows,retained.size());
    std::puts("PASS NPC table cache reuse, returned-string lifetime after local reader/archive release, malformed extent/data rollback and scene-heap cleanup");
    std::puts("PASS original NPC archive identity and eager preparation before stationed-heap trim; subsequent lookups allocate no owner");
}
