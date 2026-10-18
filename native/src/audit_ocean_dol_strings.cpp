#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cassert>
#include <cstring>
unsigned be(const unsigned char*p){return unsigned(p[0])<<24|unsigned(p[1])<<16|unsigned(p[2])<<8|p[3];}
int main(int argc,char**argv){assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::vector<unsigned char>b((std::istreambuf_iterator<char>(f)),{});assert(b.size()>256);const char* target="OceanRingPlanetLowInWater";unsigned hits=0;for(size_t pos=0;pos+strlen(target)<b.size();++pos){if(memcmp(b.data()+pos,target,strlen(target)))continue;++hits;size_t start=pos>256?pos-256:0;while(start<pos&&b[start-1]!=0)++start;for(size_t off=start;off<pos+strlen(target)+180&&off<b.size();){if(!b[off]){++off;continue;}size_t end=off;while(end<b.size()&&b[end])++end;unsigned address=0;for(unsigned i=0;i<18;++i){auto file=be(b.data()+i*4),size=be(b.data()+0x90+i*4);if(off>=file&&off<file+size)address=be(b.data()+0x48+i*4)+unsigned(off-file);}printf("offset %08zx address %08x bytes ",off,address);for(size_t j=off;j<end;++j)printf("%02x",b[j]);printf(" ascii ");for(size_t j=off;j<end;++j)putchar(b[j]>=32&&b[j]<127?b[j]:'.');putchar('\n');off=end+1;}}assert(hits);printf("Located %u exact ocean model string occurrences; neighboring names are evidence candidates, not yet verified call targets.\n",hits);}
