#include <Game/System/SaveDataHandler.hpp>
#include <Game/System/SaveDataFileAccessor.hpp>
#include <cstring>
void SaveDataHandler::copySaveDataEachFile(u8* destination,const u8* source) {
 SaveDataFileAccessor src(const_cast<u8*>(source)),dst(destination);
 for(u32 i=0;i<src.getHeader()->mUserFileInfoNum;++i){
 SaveDataUserFileInfo a,b;const char* name=src.getFileInfo(i)->mName;src.makeUserFileInfo(&a,name);dst.makeUserFileInfo(&b,name);
 if(a.mData&&b.mData){u32 count=a.mDataSize<b.mDataSize?a.mDataSize:b.mDataSize;std::memcpy(b.mData,a.mData,count);if(count<b.mDataSize)std::memset(b.mData+count,0,b.mDataSize-count);}
 }
}

