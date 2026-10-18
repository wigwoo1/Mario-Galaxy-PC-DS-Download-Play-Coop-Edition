bool SaveDataHandler::requestVerifyAfterLoadGameDataFile() {
 std::vector<u8> memory;
 if(_C>0x10000||!Native::decodeSaveEnvelope(_14,_C,memory))return false;
 std::memcpy(_14,memory.data(),memory.size());
 copySaveDataEachFile(_18,_14);return true;
}
void SaveDataHandler::requestSaveSaveData() {
 std::vector<u8> disk;
 if(!Native::encodeSaveEnvelope(_18,0x10000,disk)){mNANDRequestInfo->mResult=NAND_RESULT_CORRUPT;return;}
 std::memcpy(_14,disk.data(),disk.size());
 mNANDRequestInfo->setWriteSeq(::cSaveFileName,_14,u32(disk.size()),60,0);
 MR::addRequestToNANDManager(mNANDRequestInfo);
 setNerve(GET_NERVE_DIRECT(NativeSaveHandlerNerves,SaveDataHandlerSaveProcessingGameData));
}
void SaveDataHandler::copySaveDataEachFile(u8* destination,const u8* source) {
 SaveDataFileAccessor src(const_cast<u8*>(source)),dst(destination);
 for(u32 i=0;i<src.getHeader()->mUserFileInfoNum;++i){
 SaveDataUserFileInfo a,b;const char* name=src.getFileInfo(i)->mName;src.makeUserFileInfo(&a,name);dst.makeUserFileInfo(&b,name);
 if(a.mData&&b.mData){u32 count=a.mDataSize<b.mDataSize?a.mDataSize:b.mDataSize;std::memcpy(b.mData,a.mData,count);if(count<b.mDataSize)std::memset(b.mData+count,0,b.mDataSize-count);}
 }
}
void SaveDataHandler::restoreGameDataFile(const char* name,void* output,u32 capacity) {
 SaveDataFileAccessor accessor(_18);SaveDataUserFileInfo info;accessor.makeUserFileInfo(&info,name);
 if(output&&info.mData&&capacity>=info.mDataSize)std::memcpy(output,info.mData,info.mDataSize);
}
