#ifndef NATIVE_RFL_ARCHIVE_H
#define NATIVE_RFL_ARCHIVE_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#define NATIVE_RFL_SECTION_COUNT 18
typedef struct NativeRflSection {uint16_t count,largest;uint32_t table;} NativeRflSection;
typedef struct NativeRflHeader {uint16_t version;NativeRflSection sections[NATIVE_RFL_SECTION_COUNT];} NativeRflHeader;
static inline uint16_t nativeRflU16(const uint8_t* p){return (uint16_t)((uint16_t)p[0]<<8|p[1]);}
static inline uint32_t nativeRflU32(const uint8_t* p){return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
static inline int nativeRflSection(const void* data,size_t size,unsigned index,NativeRflSection* out,size_t* payload,size_t* limit){
 const uint8_t* p=(const uint8_t*)data;
 if(!p||!out||!payload||!limit||size<4+4*NATIVE_RFL_SECTION_COUNT||size>UINT32_MAX||index>=NATIVE_RFL_SECTION_COUNT||nativeRflU16(p)!=NATIVE_RFL_SECTION_COUNT)return 0;
 size_t start=nativeRflU32(p+4+index*4);
 size_t end=index+1<NATIVE_RFL_SECTION_COUNT?nativeRflU32(p+8+index*4):size;
 if(start<4+4*NATIVE_RFL_SECTION_COUNT||end>size||start>end||end-start<8)return 0;
 NativeRflSection section;section.count=nativeRflU16(p+start);section.largest=nativeRflU16(p+start+2);section.table=(uint32_t)(start+4);
 size_t tableSize=((size_t)section.count+1)*4;
 if(tableSize>end-start-4)return 0;
 *payload=start+4+tableSize;*limit=end;*out=section;return 1;
}
static inline int nativeRflFileSpan(const void* data,size_t size,unsigned sectionIndex,unsigned fileIndex,uint32_t* offset,uint32_t* length){
 NativeRflSection section;size_t payload,limit;const uint8_t* p=(const uint8_t*)data;
 if(!offset||!length||!nativeRflSection(data,size,sectionIndex,&section,&payload,&limit)||fileIndex>=section.count)return 0;
 uint32_t self=nativeRflU32(p+section.table+fileIndex*4),next=nativeRflU32(p+section.table+(fileIndex+1)*4);
 if(self>next||next>limit-payload||next-self>section.largest)return 0;
 *offset=(uint32_t)(payload+self);*length=next-self;return 1;
}
static inline int nativeRflParse(const void* data,size_t size,NativeRflHeader* out){
 NativeRflHeader candidate;const uint8_t* p=(const uint8_t*)data;
 if(!out)return 0;
 memset(&candidate,0,sizeof(candidate));
 for(unsigned i=0;i<NATIVE_RFL_SECTION_COUNT;++i){
  size_t payload,limit;
  if(!nativeRflSection(data,size,i,&candidate.sections[i],&payload,&limit))return 0;
  const NativeRflSection* section=&candidate.sections[i];
  if(nativeRflU32(p+section->table)!=0)return 0;
  for(unsigned f=0;f<section->count;++f){uint32_t offset,length;if(!nativeRflFileSpan(data,size,i,f,&offset,&length))return 0;}
 }
 candidate.version=nativeRflU16(p+2);*out=candidate;return 1;
}
#endif
