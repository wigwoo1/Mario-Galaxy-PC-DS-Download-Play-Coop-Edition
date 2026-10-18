#include "Game/System/BinaryDataChunkHolder.hpp"
#include <vector>
#include <cstring>
#include <stdexcept>
namespace {
u32 readWord(const u8* p){return u32(p[0])<<24|u32(p[1])<<16|u32(p[2])<<8|p[3];}
void writeWord(u8* p,u32 x){p[0]=x>>24;p[1]=x>>16;p[2]=x>>8;p[3]=x;}
}
BinaryDataChunkHolder::BinaryDataChunkHolder(u32 bytes,int count):mChunks(nullptr),mMaxChunks(count),mNumChunks(0),mData(nullptr),mBufferSize(bytes){if(count<0||count>255||bytes<12)throw std::invalid_argument("Invalid save chunk capacity");mData=new u8[bytes]{};mChunks=new BinaryDataChunkBase*[count]{};}
void BinaryDataChunkHolder::addChunk(BinaryDataChunkBase* chunk){if(!chunk||mNumChunks>=mMaxChunks)throw std::invalid_argument("Invalid save chunk registration");mChunks[mNumChunks++]=chunk;}
BinaryDataChunkBase* BinaryDataChunkHolder::findFromSignature(u32 signature) const {for(int i=0;i<mNumChunks;++i)if(mChunks[i]->getSignature()==signature)return mChunks[i];return nullptr;}
void BinaryDataChunkHolder::makeChunkData(BinaryDataChunkHolderChunkData* data,u32 capacity,const BinaryDataChunkBase* chunk){if(!data||!chunk||capacity<12)throw std::invalid_argument("Invalid save chunk buffer");auto bytes=reinterpret_cast<u8*>(data);auto size=chunk->serialize(bytes+12,capacity-12);if(size<0||u32(size)>capacity-12)throw std::runtime_error("Invalid serialized save chunk size");writeWord(bytes,chunk->getSignature());writeWord(bytes+4,chunk->makeHeaderHashCode());writeWord(bytes+8,u32(size)+12);}
u32 BinaryDataChunkHolder::makeFileBinary(u8* output,u32 capacity){if(!output||capacity<4)return 0;std::vector<u8> file{1,u8(mNumChunks),0,0};for(int i=0;i<mNumChunks;++i){std::memset(mData,0,mBufferSize);makeChunkData(static_cast<BinaryDataChunkHolderChunkData*>(mData),mBufferSize,mChunks[i]);auto bytes=static_cast<u8*>(mData);u32 size=readWord(bytes+8);if(size>capacity-file.size())return 0;file.insert(file.end(),bytes,bytes+size);}std::memcpy(output,file.data(),file.size());return u32(file.size());}
bool BinaryDataChunkHolder::loadFromFileBinary(const u8* data,u32 size){if(!data||size<4||data[0]!=1)return false;struct Entry{BinaryDataChunkBase* chunk;u32 offset,size;};std::vector<Entry> entries;u32 offset=4;for(u32 i=0;i<data[1];++i){if(offset>size||size-offset<12)return false;u32 length=readWord(data+offset+8);if(length<12||length>size-offset)return false;auto chunk=findFromSignature(readWord(data+offset));if(chunk){if(readWord(data+offset+4)!=chunk->makeHeaderHashCode())return false;for(auto entry:entries)if(entry.chunk==chunk)return false;entries.push_back({chunk,offset+12,length-12});}offset+=length;}for(auto entry:entries){s32 result=entry.chunk->deserialize(data+entry.offset,entry.size);if(result!=0&&result!=1)return false;}return true;}
// Legacy interface has no buffer length; callers must supply a complete file.
u32 BinaryDataChunkHolder::calcBinarySize(const u8* data){if(!data)return 0;u32 offset=4;for(u32 i=0;i<data[1];++i){u32 size=readWord(data+offset+8);if(size<12||size>0xffffffffu-offset)return 0;offset+=size;}return offset;}
