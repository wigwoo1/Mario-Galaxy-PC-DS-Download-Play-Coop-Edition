#include <Game/System/SaveDataHandler.hpp>
#include <Game/System/SaveDataFileAccessor.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
namespace MR {bool isEqualString(const char* a,const char* b){return std::strcmp(a,b)==0;}}
void setup(u8* p,u32 size){auto h=reinterpret_cast<SaveDataFileHeader*>(p);h->mUserFileInfoNum=1;h->mFileSize=size;auto i=reinterpret_cast<SaveDataFileInfo*>(p+16);std::strcpy(i->mName,"mario1");i->mOffset=32;}
int main(){alignas(SaveDataFile)u8 source[64]{},destination[64]{};setup(source,36);setup(destination,40);source[32]=1;source[33]=2;source[34]=3;source[35]=4;std::memset(destination+32,0xaa,32);SaveDataHandler::copySaveDataEachFile(destination,source);assert(!std::memcmp(source+32,destination+32,4));for(int i=36;i<40;++i)assert(destination[i]==0);assert(destination[40]==0xaa);setup(destination,34);std::memset(destination+32,0xaa,32);SaveDataHandler::copySaveDataEachFile(destination,source);assert(destination[32]==1&&destination[33]==2&&destination[34]==0xaa);std::puts("PASS save-handler slot copying: short source zero fill, short destination clamp and adjacent-byte preservation");}
