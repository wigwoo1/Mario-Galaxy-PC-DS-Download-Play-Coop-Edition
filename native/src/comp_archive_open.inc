{
 mInfoBlock=nullptr;mDirs=nullptr;mFiles=nullptr;mStringTable=nullptr;mExpandSizes=nullptr;
 field_0x64=0;mAramPart=nullptr;field_0x6c=0;mDvdFile=nullptr;
 mSizeOfMemPart=0;mSizeOfAramPart=0;field_0x7c=0;
 auto fail=[&]{delete mAramPart;mAramPart=nullptr;delete mDvdFile;mDvdFile=nullptr;if(mExpandSizes){JKRHeap::free(mExpandSizes,mHeap);mExpandSizes=nullptr;}nativeDetachRarcMetadata(this);field_0x64=0;mMountMode=MOUNT_MODE_0;return false;};
 std::vector<uint8_t> bytes;
 if(!nativeReadDvdEntry(entryNum,bytes) || !nativeRegisterRarc(bytes.data(),bytes.size()))return fail();
 bool attached=nativeAttachRarcMetadata(this,bytes.data());nativeUnregisterRarc(bytes.data());
 if(!attached)return fail();
 auto tables=nativeOwnedRarcTables(this);if(!tables)return fail();
 try{for(const auto& file:tables->files)if(file.mFlag&FILE_FLAG_FILE)tables->filePartition(file);}catch(const std::exception&){return fail();}
 _5C=bytes.size()>=4 && !std::memcmp(bytes.data(),"Yaz0",4)?2:bytes.size()>=4 && !std::memcmp(bytes.data(),"Yay0",4)?1:0;
 auto main=tables->mainRamPartition(),aram=tables->aramPartition();
 mSizeOfMemPart=u32(main.size);mSizeOfAramPart=u32(aram.size);
 field_0x64=reinterpret_cast<std::uintptr_t>(tables->bytes.data()+main.offset);
 field_0x6c=int(tables->payload);
 void* storage=JKRHeap::alloc(sizeof(JKRDvdFile),32,JKRHeap::getSystemHeap());
 mDvdFile=storage?new(storage) JKRDvdFile(entryNum):nullptr;
 if(!mDvdFile || !mDvdFile->isAvailable())return fail();
 if(aram.size){
  if(!JKRAram::sAramObject || aram.size>0xffffffe0u)return fail();
  size_t rounded=(aram.size+31)&~size_t(31);
  mAramPart=JKRAram::sAramObject->mAramHeap->alloc(u32(rounded),JKRAramHeap::HEAD);
  if(!mAramPart)return fail();
  std::vector<uint8_t> padded(rounded);std::memcpy(padded.data(),tables->bytes.data()+aram.offset,aram.size);
  if(!nativeUploadAram(mAramPart->getAddress(),padded.data(),padded.size()))return fail();
 }
 bool compressed=false;for(const auto& file:tables->files)if((file.mFlag&FILE_FLAG_FILE) && !(file.mFlag&FILE_FLAG_MRAM))compressed|=(file.mFlag&FILE_FLAG_COMPRESSED)!=0;
 if(compressed){mExpandSizes=static_cast<s32*>(JKRHeap::alloc(mInfoBlock->mNrFiles*sizeof(s32),32,mHeap));if(!mExpandSizes)return fail();std::memset(mExpandSizes,0,mInfoBlock->mNrFiles*sizeof(s32));}
 return true;
}
