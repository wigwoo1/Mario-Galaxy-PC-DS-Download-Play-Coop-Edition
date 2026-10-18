#pragma once
#include <Game/Util/JMapInfo.hpp>
// Binding lifetime must cover all stage arrays: table bytes and names are retained.
class NativeStageArchiveTables {
public:
 virtual ~NativeStageArchiveTables()=default;
 virtual NativeStageArchiveTables* resolveStage(const char* name)=0;
 virtual const JMapInfo* objectNameTable()=0;
 virtual void* resource(const char* name)=0;
 virtual s32 resourceSize(const void* data)=0;
 virtual s32 countTables(const char* directory)=0;
 virtual JMapInfo* appendTables(JMapInfo* destination,const char* directory)=0;
};
#include <Game/Util/Array.hpp>
inline bool nativeStageArrayOwns(const MR::AssignableArray<JMapInfo>& tables,const JMapInfoIter& iter){
 if(!iter.mInfo || !iter.mInfo->mData || iter.mIndex<0 || iter.mIndex>=iter.mInfo->getNumEntries())return false;
 for(s32 i=0;i<tables.mMaxSize;++i)if(tables.mArr[i].mData==iter.mInfo->mData)return true;
 return false;
}



