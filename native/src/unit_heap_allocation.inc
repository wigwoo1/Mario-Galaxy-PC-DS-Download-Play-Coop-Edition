void* JKRUnitHeap::do_alloc(u32 size,int alignment){
 lock();
 const int64_t requested=alignment<0?-int64_t(alignment):int64_t(alignment);
 void* result=nullptr;
 if(size<=mUnitSize && requested<=mAlignment){
  const s32 index=find1FreeBlock(alignment);
  if(index>=0 && u32(index)<mUnitCount){setUnitUsed(index);mTotalFreeSize-=mUnitSize;result=indexToAddress(index);}
 }
 unlock();
 if(!result && mErrorFlag)callErrorHandler(this,size,alignment);
 return result;
}
void JKRUnitHeap::do_free(void* address){
 if(!address)return;
 lock();
 const s32 index=addressToIndex(address);
 if(index>=0 && isUnitUsed(index)){
  dispose(address,mUnitSize);
  mBat[index/8]&=u8(~(0x80u>>(index&7)));
  mTotalFreeSize+=mUnitSize;
 }
 unlock();
}
