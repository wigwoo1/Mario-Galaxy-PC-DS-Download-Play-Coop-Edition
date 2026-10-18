#include <fstream>
#include <vector>
#include <cstdio>
#include <cassert>
#include <cstdint>
unsigned be(const unsigned char*p){return unsigned(p[0])<<24|unsigned(p[1])<<16|unsigned(p[2])<<8|p[3];}
int main(int argc,char**argv){assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::vector<unsigned char>b((std::istreambuf_iterator<char>(f)),{});assert(b.size()>256);unsigned hits=0;for(unsigned s=0;s<7;++s){auto file=be(b.data()+s*4),size=be(b.data()+0x90+s*4),base=be(b.data()+0x48+s*4);assert(uint64_t(file)+size<=b.size());for(unsigned off=0;off+4<=size;off+=4){unsigned w=be(b.data()+file+off);if((w>>26)!=15||((w>>16)&31)!=0||(w&65535)!=0x8057)continue;unsigned reg=(w>>21)&31;for(unsigned step=4;step<=32&&off+step+4<=size;step+=4){unsigned next=be(b.data()+file+off+step);if((next>>26)!=14||((next>>16)&31)!=reg)continue;unsigned address=0x80570000+int16_t(next&65535);if(address!=0x805766f0&&address!=0x80576741&&address!=0x80576750&&address!=0x80576709&&address!=0x80576727)continue;++hits;printf("PPC reference %08x lis r%u,8057 + addi at %08x => %08x\n",base+off,reg,base+off+step,address);unsigned begin=off>16?off-16:0;for(unsigned pos=begin;pos<off+96&&pos+4<=size;pos+=4)printf("%08x %08x\n",base+pos,be(b.data()+file+pos));}}}assert(hits);printf("Matched %u immediate address constructions.\n",hits);}
