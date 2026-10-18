{
 mInfoBlock=nullptr;mDirs=nullptr;mFiles=nullptr;mStringTable=nullptr;mExpandSizes=nullptr;mFile=nullptr;_64=0;
 std::vector<uint8_t> bytes;
 if(!nativeReadDvdEntry(entryNum,bytes) || !nativeRegisterRarc(bytes.data(),bytes.size())){mMountMode=MOUNT_MODE_0;return false;}
 std::size_t payload=0;
 bool attached=nativeAttachRarcMetadata(this,bytes.data(),&payload);
 nativeUnregisterRarc(bytes.data());
 if(!attached){mMountMode=MOUNT_MODE_0;return false;}
 _5C=bytes.size()>=4 && !std::memcmp(bytes.data(),"Yaz0",4)?2:bytes.size()>=4 && !std::memcmp(bytes.data(),"Yay0",4)?1:0;
 _64=static_cast<u32>(payload);
 void* fileStorage=JKRHeap::alloc(sizeof(JKRDvdFile),32,JKRHeap::getSystemHeap());
 mFile=fileStorage?new (fileStorage) JKRDvdFile(entryNum):nullptr;
 if(!mFile || !mFile->isAvailable()){delete mFile;mFile=nullptr;nativeDetachRarcMetadata(this);mMountMode=MOUNT_MODE_0;return false;}
 bool compressed=false;
 for(u32 i=0;i<mInfoBlock->mNrFiles;++i)if(mFiles[i].mFlag&FILE_FLAG_FILE)compressed|=(mFiles[i].mFlag&FILE_FLAG_COMPRESSED)!=0;
 if(compressed){mExpandSizes=static_cast<s32*>(JKRHeap::alloc(mInfoBlock->mNrFiles*sizeof(s32),mMountDir==MOUNT_DIRECTION_1?32:-32,mHeap));if(!mExpandSizes){delete mFile;mFile=nullptr;nativeDetachRarcMetadata(this);mMountMode=MOUNT_MODE_0;return false;}std::memset(mExpandSizes,0,mInfoBlock->mNrFiles*sizeof(s32));}
 return true;
}

