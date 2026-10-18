#include <cstring>
namespace {
u64 sysRead(const u8* p,int n){u64 x=0;for(int i=0;i<n;++i)x=(x<<8)|p[i];return x;}
void sysWrite(u8* p,u64 x,int n){for(int i=n-1;i>=0;--i){p[i]=u8(x);x>>=8;}}
}
s32 SysConfigChunk::serialize(u8* output,u32 capacity) const {
 const u32 header=mHeaderSerializer->getHeaderSize(),data=mHeaderSerializer->getDataSize();
 if(!output||capacity<header||capacity-header<data)return -1;
 auto original=static_cast<u8*>(mHeaderSerializer->mStream.mBuffer);BinaryDataContentAccessor accessor(original);
 u8* announced=static_cast<u8*>(accessor.getPointer("mTimeAnnounced",output+header));
 u8* sent=static_cast<u8*>(accessor.getPointer("mTimeSent",output+header));
 u8* bytes=static_cast<u8*>(accessor.getPointer("mSentBytes",output+header));
 if(!announced||!sent||!bytes)return -1;
 std::memcpy(output,original,header);sysWrite(announced,u64(mTimeAnnounced),8);sysWrite(sent,u64(mTimeSent),8);sysWrite(bytes,mSentBytes,4);return header+data;
}
s32 SysConfigChunk::deserialize(const u8* input,u32 size) {
 if(!input||size<4)return 3;
 u32 header=u32(sysRead(input,2))*4+4,data=u32(sysRead(input+2,2));
 if(header>size||data>size-header)return 3;
 BinaryDataContentAccessor accessor(const_cast<u8*>(input));u8* payload=const_cast<u8*>(input)+header;
 auto announced=static_cast<u8*>(accessor.getPointer("mTimeAnnounced",payload));auto sent=static_cast<u8*>(accessor.getPointer("mTimeSent",payload));auto bytes=static_cast<u8*>(accessor.getPointer("mSentBytes",payload));
 if(!announced||!sent||!bytes||announced-payload>data||data-u32(announced-payload)<8||sent-payload>data||data-u32(sent-payload)<8||bytes-payload>data||data-u32(bytes-payload)<4)return 3;
 mTimeAnnounced=OSTime(sysRead(announced,8));mTimeSent=OSTime(sysRead(sent,8));mSentBytes=u32(sysRead(bytes,4));return 0;
}
