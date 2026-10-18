#include <Game/RhythmLib/AudMePlayer.hpp>
#include <map>
#include <memory>
#include <vector>
#include <mutex>
#include <stdexcept>
namespace {
u32 meWord(const u8* p){return u32(p[0])<<24|u32(p[1])<<16|u32(p[2])<<8|p[3];}
struct Tables {std::vector<AudMePlayingParams> params;std::vector<const char*> names;};
std::map<AudMePlayingParamsHolder*,std::unique_ptr<Tables>> storage;
std::mutex tableLock;
}
void AudMePlayingParamsHolder::setResource(void* resource){
 std::lock_guard<std::mutex> guard(tableLock);
 if(!resource){storage.erase(this);mNumEntries=0;mParams=nullptr;mNames=nullptr;return;}
 auto bytes=static_cast<const u8*>(resource);u32 count=meWord(bytes),entryOffset=meWord(bytes+4),nameOffset=meWord(bytes+8);
 if(count>65536||entryOffset<12||nameOffset<12)throw std::invalid_argument("Invalid music-effect table header");
 auto tables=std::make_unique<Tables>();tables->params.resize(count);tables->names.resize(count);
 for(u32 i=0;i<count;++i){auto p=bytes+entryOffset+i*8;auto& v=tables->params[i];v.mPriority=p[0];v.mChordVolume=p[1];v._2=p[2];v._3=p[3];v._4=u16(p[4])<<8|p[5];v._6=p[6];v._7=p[7];tables->names[i]=reinterpret_cast<const char*>(bytes+meWord(bytes+nameOffset+i*4));}
 auto params=tables->params.data();auto names=tables->names.data();storage[this]=std::move(tables);mNumEntries=count;mParams=params;mNames=names;
}
