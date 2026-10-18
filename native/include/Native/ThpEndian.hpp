#pragma once
#include <revolution/thp.h>
#include <cstring>
namespace NativeThp {
inline u32 read32(const void* source){const u8* p=static_cast<const u8*>(source);return (u32(p[0])<<24)|(u32(p[1])<<16)|(u32(p[2])<<8)|p[3];}
inline void header(THPHeader& h,const void* source){const u8* p=static_cast<const u8*>(source);std::memcpy(h.magic,p,4);h.version=read32(p+4);h.bufSize=read32(p+8);h.audioMaxSamples=read32(p+12);u32 rate=read32(p+16);std::memcpy(&h.frameRate,&rate,4);h.numFrames=read32(p+20);h.firstFrameSize=read32(p+24);h.movieDataSize=read32(p+28);h.compInfoDataOffsets=read32(p+32);h.offsetDataOffsets=read32(p+36);h.movieDataOffsets=read32(p+40);h.finalFrameDataOffsets=read32(p+44);}
inline void components(THPFrameCompInfo& c,const void* source){const u8* p=static_cast<const u8*>(source);c.numComponents=read32(p);std::memcpy(c.frameComp,p+4,16);}
inline void video(THPVideoInfo& v,const void* source){const u8* p=static_cast<const u8*>(source);v.xSize=read32(p);v.ySize=read32(p+4);v.videoType=read32(p+8);}
inline void audio(THPAudioInfo& a,const void* source){const u8* p=static_cast<const u8*>(source);a.sndChannels=read32(p);a.sndFrequency=read32(p+4);a.sndNumSamples=read32(p+8);a.sndNumTracks=read32(p+12);}
// Match each separately aligned region assigned by the original setBuffer.
inline u32 workBytes(const THPHeader& h,const THPVideoInfo& v,bool audio){
 if(!h.bufSize||h.bufSize>0x7fffffffU||!v.xSize||!v.ySize||(v.xSize&1)||(v.ySize&1))return 0;
 const u64 pixels=u64(v.xSize)*v.ySize;
 auto align=[](u64 n){return (n+31)&~u64(31);};
 // Reject huge dimensions before any subsequent multiplication can overflow.
 if(pixels>0xffffffffULL)return 0;
 u64 total=align(h.bufSize)*20+align(pixels)*2+align(pixels/4)*4+0x1000;
 if(audio)total+=align(u64(h.audioMaxSamples)*4)*20;
 return total<=0xffffffffULL?u32(total):0;
}
}

