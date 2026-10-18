#include "asset_tests.hpp"
#include "owned_archive_resources.hpp"
#include <cassert>
#include <cstdio>
int main(){
 assetTests();
 std::vector<uint8_t> s={'Y','a','y','0',0,0,0,6,0,0,0,20,0,0,0,22,0xC0,0,0,0,0x20,1,'X','Y'};
 assert(decodeYay0(s)==std::vector<uint8_t>({'X','Y','X','Y','X','Y'}));
 auto reject=[](const std::vector<uint8_t>& v){bool caught=false;try{decodeYay0(v);}catch(const std::runtime_error&){caught=true;}assert(caught);};
 for(size_t i=0;i<s.size();++i)reject(std::vector<uint8_t>(s.begin(),s.begin()+i));
 auto bad=s;bad[21]=9;reject(bad);bad=s;bad[7]=5;reject(bad);
 bad=s;bad[11]=15;reject(bad);bad=s;bad[15]=21;reject(bad);
 std::vector<uint8_t> arc(0xa0,0); std::memcpy(arc.data(),"RARC",4);
 put32(arc,4,0xa0);put32(arc,8,0x20);put32(arc,12,0x60);put32(arc,16,0x20);
 put32(arc,0x20,1);put32(arc,0x24,0x20);put32(arc,0x28,1);put32(arc,0x2c,0x30);
 put32(arc,0x30,2);put32(arc,0x34,0x44);arc[0x4b]=1;
 put32(arc,0x54,0x01000000);put32(arc,0x5c,3);arc[0x64]='f';
 arc[0x80]='a';arc[0x81]='b';arc[0x82]='c';
 size_t maskBytes=((arc.size()+31)/32)*4;
 std::vector<uint8_t> packed(16+maskBytes,255);
 std::memcpy(packed.data(),"Yay0",4);put32(packed,4,uint32_t(arc.size()));
 put32(packed,8,uint32_t(packed.size()));put32(packed,12,uint32_t(packed.size()));
 packed.insert(packed.end(),arc.begin(),arc.end());
 OwnedArchiveResources owned(packed);
 const uint8_t* resource=owned.get("/f"); assert(resource && owned.sizeOf(resource)==3);
 assert(std::memcmp(resource,"abc",3)==0 && owned.get("/f")==resource);
 assert(owned.get("/missing")==nullptr && owned.get("/f")==resource);
 Rarc loaded(packed);assert(loaded.readFile("/f")==std::vector<uint8_t>({'a','b','c'}));
 bool failed=false;packed.pop_back();try{Rarc broken(packed);}catch(const std::runtime_error&){failed=true;}assert(failed);
 auto embedded=arc;put32(embedded,0x5c,uint32_t(s.size()));std::copy(s.begin(),s.end(),embedded.begin()+0x80);
 OwnedArchiveResources compressed(embedded);const auto* expanded=compressed.get("/f");
 assert(expanded && compressed.sizeOf(expanded)==6 && std::memcmp(expanded,"XYXYXY",6)==0);
 uint8_t dest[8]={0,0,0,0,0,0,0xAA,0xBB};
 assert(compressed.read(dest,4,"/f")==4 && std::memcmp(dest,"XYXY",4)==0 && dest[4]==0 && dest[6]==0xAA);
 assert(compressed.read(dest,8,"/f")==6 && dest[6]==0xAA && dest[7]==0xBB);
 assert(compressed.read(nullptr,0,"/f")==0 && compressed.read(dest,8,"/missing")==0);
 failed=false;try{compressed.read(nullptr,1,"/f");}catch(const std::runtime_error&){failed=true;}assert(failed);
 failed=false;try{compressed.sizeOf(dest);}catch(const std::runtime_error&){failed=true;}assert(failed);
 assert(compressed.get("/f")==expanded);
 std::puts("Bounded Yay0 PASS: overlap, all truncations, invalid offsets/distance/output length");
}




