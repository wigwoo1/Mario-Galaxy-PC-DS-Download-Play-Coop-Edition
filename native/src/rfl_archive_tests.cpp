#include "native_disc_archive_loader.hpp"
#include <cassert>
#include <cstdio>
#include <Native/RflArchive.h>
static uint32_t read(const uint8_t* p,unsigned n){uint32_t value=0;while(n--)value=value*256+*p++;return value;}
static void write(std::vector<uint8_t>& bytes,size_t at,uint32_t value){for(int i=3;i>=0;--i){bytes[at+i]=uint8_t(value);value>>=8;}}
static void rejects(const std::vector<uint8_t>& bytes){
 NativeRflHeader result;std::memset(&result,0xA5,sizeof(result));NativeRflHeader before=result;
 assert(!nativeRflParse(bytes.data(),bytes.size(),&result));assert(std::memcmp(&before,&result,sizeof(result))==0);
}
int main(int argc,char** argv){
 assert(argc==2);NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));
 auto archive=loader.mount("/ObjectData/MiiFaceDatabase.arc");
 const auto* resource=archive->get("/rfl_res.dat");
 if(!resource){for(const auto& file:archive->files())std::puts(file.path.c_str());return 2;}
 const auto size=archive->sizeOf(resource);const std::vector<uint8_t> original(resource,resource+size);
 NativeRflHeader header;assert(nativeRflParse(resource,size,&header));
 assert(size==686372&&header.version==0x039D&&read(resource,2)==18);
 size_t files=0,payloadBytes=0;
 for(unsigned i=0;i<18;++i){
  const size_t section=read(resource+4+4*i,4);const unsigned count=read(resource+section,2),largest=read(resource+section+2,2);
  assert(header.sections[i].count==count&&header.sections[i].largest==largest&&header.sections[i].table==section+4);
  for(unsigned f=0;f<count;++f){
   uint32_t offset=0,length=0;assert(nativeRflFileSpan(resource,size,i,f,&offset,&length));
   const size_t self=read(resource+section+4+4*f,4),next=read(resource+section+8+4*f,4);
   assert(length==next-self&&offset==section+4+(count+1)*4+self&&offset+length<=size);
   std::vector<uint8_t> copy(resource+offset,resource+offset+length);
   assert(std::equal(copy.begin(),copy.end(),original.begin()+offset));++files;payloadBytes+=length;
  }
  uint32_t offset=0xABCD,length=0xEF01;assert(!nativeRflFileSpan(resource,size,i,count,&offset,&length));assert(offset==0xABCD&&length==0xEF01);
 }
 assert(std::memcmp(resource,original.data(),size)==0);
 for(size_t length=0;length<76;++length)rejects(std::vector<uint8_t>(original.begin(),original.begin()+length));
 auto bad=original;write(bad,4,uint32_t(size+1));rejects(bad);
 bad=original;write(bad,4,0);rejects(bad);
 const size_t first=read(resource+4,4);
 bad=original;bad[first]=bad[first+1]=0xFF;rejects(bad);
 bad=original;write(bad,first+4,1);rejects(bad);
 bad=original;write(bad,first+8,UINT32_MAX);rejects(bad);
 bad=original;write(bad,first+12,0);rejects(bad);
 bad=original;bad[first+2]=bad[first+3]=0;rejects(bad);
 bad=original;bad[0]=0;bad[1]=17;rejects(bad);
 uint32_t offset,length;assert(!nativeRflFileSpan(resource,size,18,0,&offset,&length));
 assert(!nativeRflFileSpan(nullptr,size,0,0,&offset,&length));
 std::printf("PASS real RFL archive: %zu bytes,18 sections,%zu files,%zu payload bytes match independent disk reader; unchanged payloads, bounded offsets and atomic malformed-input rejection\n",size,files,payloadBytes);
}
