#include <Game/RhythmLib/AudChordInfo.hpp>
#include <map>
#include <memory>
#include <vector>
#include <mutex>
#include <cstring>
namespace {
u32 chordWord(const u8* p){return u32(p[0])<<24|u32(p[1])<<16|u32(p[2])<<8|p[3];}
u16 chordShort(const u8* p){return u16(p[0])<<8|p[1];}
struct Tables {std::vector<AudChordData*> chords;std::vector<AudScaleData> scales;std::vector<AudScaleData*> pointers;};
std::map<AudChordTable*,std::unique_ptr<Tables>> tables;
std::mutex chordLock;
}
AudChordTable::AudChordTable():mLoaded(false),mChordCount(0),mScaleCount(0),mChordPtr(nullptr),mScalePtr(nullptr){}
bool AudChordTable::setChordTableResource(void* resource){
 std::lock_guard<std::mutex> guard(chordLock);if(!resource)return false;auto bytes=static_cast<u8*>(resource);
 if(chordWord(bytes)!=0||std::memcmp(bytes+4,"CITS",4))return false;
 u32 chords=chordShort(bytes+12),scales=chordShort(bytes+14);auto data=std::make_unique<Tables>();data->chords.resize(chords);data->scales.resize(scales);data->pointers.resize(scales);
 for(u32 i=0;i<chords;++i)data->chords[i]=reinterpret_cast<AudChordData*>(bytes+chordWord(bytes+16+i*4));
 for(u32 i=0;i<scales;++i){auto scale=bytes+chordWord(bytes+16+chords*4+i*4);data->scales[i].up=bytes+chordWord(scale);data->scales[i].down=bytes+chordWord(scale+4);data->pointers[i]=&data->scales[i];}
 auto chordPointers=data->chords.data();auto scalePointers=data->pointers.data();tables[this]=std::move(data);mChordCount=chords;mScaleCount=scales;mChordPtr=chordPointers;mScalePtr=scalePointers;return true;
}
