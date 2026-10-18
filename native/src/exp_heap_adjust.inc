// Native adaptation: keep one trailing free block so allocator metadata remains
// inside retained storage. Caller must quiesce allocation and child creation.
s32 JKRExpHeap::adjustSize(){
    if(mIsExternal || !mChildTree.getParent())return -1;
    auto parent=mChildTree.getParent()->getObject();
    if(!parent || parent->getHeapType()!=getHeapType())return -1;
    lock();
    auto tail=mTailFreeList;
    if(!tail || reinterpret_cast<u8*>(tail+1)+tail->mSize!=mEnd){unlock();return -1;}
    const uintptr_t retainedEnd=ALIGN_NEXT(reinterpret_cast<uintptr_t>(tail+1)+32,16);
    const uintptr_t base=reinterpret_cast<uintptr_t>(this);
    if(retainedEnd>=reinterpret_cast<uintptr_t>(mEnd) || retainedEnd-base>0x7fffffff){unlock();return -1;}
    const s32 actual=parent->resize(this,u32(retainedEnd-base));
    if(actual<0){unlock();return -1;}
    auto newEnd=reinterpret_cast<u8*>(this)+actual;
    if(newEnd>mEnd || newEnd<reinterpret_cast<u8*>(tail+1)){unlock();return -1;}
    tail->mSize=u32(newEnd-reinterpret_cast<u8*>(tail+1));
    mEnd=newEnd;mSize=u32(mEnd-mStart);
    unlock();return actual;
}
