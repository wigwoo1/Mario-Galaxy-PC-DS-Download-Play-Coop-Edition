#include "assets.hpp"
#include <Native/CameraAnimationData.hpp>
#include <cstdio>
int main(int argc,char**argv){
 if(argc!=2)return 2;WiiAssets disc(std::filesystem::u8path(argv[1]));size_t count=0,rejected=0;
 for(const auto& f:disc.files){
 if(f.path.find("DemoCamera.arc")==std::string::npos&&f.path.find("Scenario.arc")==std::string::npos)continue;
 auto bytes=disc.readFile(f.path);Rarc archive(bytes);
 for(const auto& file:archive.files){auto data=archive.readFile(file.path);if(data.size()<8||std::memcmp(data.data(),"ANDO",4))continue;
 Native::CameraAnimationData decoded;bool ok=decoded.decode(data.data(),data.size());
 std::printf("%s %s:%s bytes=%zu frames=%u format=%.4s\n",ok?"PASS":"REJECT",f.path.c_str(),file.path.c_str(),data.size(),be32(data.data()+24),data.data()+4);
 ++count;if(!ok)++rejected;
 }}
 std::printf("camera resources=%zu rejected=%zu\n",count,rejected);return count && !rejected?0:1;
}
