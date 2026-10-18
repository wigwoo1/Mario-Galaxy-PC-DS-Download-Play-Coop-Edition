{
 mInfoBlock=nullptr;mDirs=nullptr;mFiles=nullptr;mStringTable=nullptr;mBlock=nullptr;mDvdFile=nullptr;mExpandSizes=nullptr;
 auto fail=[&]{delete mBlock;mBlock=nullptr;delete mDvdFile;mDvdFile=nullptr;if(mExpandSizes){JKRHeap::free(mExpandSizes,mHeap);mExpandSizes=nullptr;}nativeDetachRarcMetadata(this);mMountMode=MOUNT_MODE_0;return false;};
 std::vector<uint8_t> bytes;
 if(!nativeReadDvdEntry(entryNum,bytes) || !nativeRegisterRarc(bytes.data(),bytes.size()))return fail();
 bool attached=nativeAttachRarcMetadata(this,bytes.data());nativeUnregisterRarc(bytes.data());if(!attached)return fail();
 auto tables=nativeOwnedRarcTables(this);if(!tables || !JKRAram::sAramObject)return fail();
 _5C=bytes.size()>=4 && !std::memcmp(bytes.data(),"Yaz0",4)?2:bytes.size()>=4 && !std::memcmp(bytes.data(),"Yay0",4)?1:0;
 size_t size=tables->header.mTotalDataSize;if(!size || size>0xffffffe0u)return fail();
 size_t rounded=(size+31)&~size_t(31);
 mBlock=JKRAram::sAramObject->mAramHeap->alloc(u32(rounded),mMountDir==MOUNT_DIRECTION_1?JKRAramHeap::HEAD:JKRAramHeap::TAIL);
 if(!mBlock)return fail();
 std::vector<uint8_t> payload(rounded);std::memcpy(payload.data(),tables->bytes.data()+tables->payload,size);
 if(!nativeUploadAram(mBlock->getAddress(),payload.data(),payload.size()))return fail();
 void* storage=JKRHeap::alloc(sizeof(JKRDvdFile),32,JKRHeap::getSystemHeap());
 mDvdFile=storage?new(storage) JKRDvdFile(entryNum):nullptr;if(!mDvdFile || !mDvdFile->isAvailable())return fail();
 bool compressed=false;for(const auto& file:tables->files)if(file.mFlag&FILE_FLAG_FILE)compressed|=(file.mFlag&FILE_FLAG_COMPRESSED)!=0;
 if(compressed){mExpandSizes=static_cast<s32*>(JKRHeap::alloc(mInfoBlock->mNrFiles*sizeof(s32),32,mHeap));if(!mExpandSizes)return fail();std::memset(mExpandSizes,0,mInfoBlock->mNrFiles*sizeof(s32));}
 return true;
}
