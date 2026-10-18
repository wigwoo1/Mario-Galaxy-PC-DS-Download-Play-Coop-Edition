#include <Game/AudioLib/AudSoundId.hpp>
#include <JSystem/JAudio2/JAUSoundTable.hpp>
#include <Game/AudioLib/AudSoundNameConverter.hpp>
#include <fstream>
#include <vector>
#include <cassert>
#include <cstdio>
int main(int argc,char** argv){assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::vector<u8> b((std::istreambuf_iterator<char>(f)),{});assert(!b.empty());JAUSoundNameTable table(true);table.init(b.data());AudSoundNameConverter converter;assert(converter.mNumItems==3487);unsigned checked=0;for(unsigned section=0;section<3;++section)for(unsigned group=0;group<unsigned(table.getNumGroups_inSection(section));++group)for(unsigned item=0;item<unsigned(table.getNumItems_inGroup(section,group));++item){JAISoundID id;id.set(section,group,item);const char* name=table.getName(id);auto resolved=converter.getSoundID(name);const u32 expected=(u32(section)<<24)|(u32(group)<<16)|u32(item);if(u32(resolved)!=expected||u32(id)!=expected){std::printf("Mismatch %s expected %08x got %08x\n",name,u32(id),u32(resolved));return 1;}++checked;}assert(u32(converter.getSoundID("STM_TITLE"))==STM_TITLE);assert(converter.getSoundID(nullptr).isAnonymous());assert(converter.getSoundID("SE").isAnonymous());assert(converter.getSoundID("SE_XX_UNKNOWN").isAnonymous());assert(converter.getSoundID("ZZ_UNKNOWN").isAnonymous());delete[] converter.mGroupItemOffsets;delete[] converter.mSoundNameData;std::printf("PASS %u retail sound-name IDs and invalid-input guards\n",checked);}
