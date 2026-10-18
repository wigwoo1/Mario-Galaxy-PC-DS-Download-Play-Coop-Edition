#include <JSystem/JAudio2/JAISound.hpp>
#include <Game/AudioLib/AudSoundId.hpp>
#include <cstdio>
#include <type_traits>
int main(){
 static_assert(sizeof(JAISoundID)==sizeof(u32));static_assert(std::is_trivially_copyable<decltype(JAISoundID::mID)>::value);
 JAISoundID raw(STM_TITLE),parts(2,0,1);
 std::printf("STM_TITLE raw=%08X decoded section=%u group=%u wave=%u; constructed=%08X\n",unsigned(u32(raw)),unsigned(raw.getSectionID()),unsigned(raw.getGroupID()),unsigned(raw.getWaveID()),unsigned(u32(parts)));
 if(raw.getSectionID()!=2||raw.getGroupID()!=0||raw.getWaveID()!=1||u32(parts)!=STM_TITLE){std::puts("FAIL sound ID byte order disagrees with original STM_TITLE numeric constant and BGM multi-mode bit");return 1;}
 if(raw.mID.info.type.parts.sectionID!=2||raw.mID.info.type.parts.groupID!=0||raw.mID.info.waveID!=1){std::puts("FAIL public sound-ID split fields disagree with canonical numeric ID");return 11;}
 const u32 cases[]={0,0x000f01a7,0x01010023,0x02000001,0xfffedcba,0xffffffff};
 for(u32 value:cases){JAISoundID id(value);if(id.mID.info.type.parts.sectionID!=u8(value>>24)||id.mID.info.type.parts.groupID!=u8(value>>16)||id.mID.info.waveID!=u16(value))return 12;if(id.getSectionID()!=u8(value>>24)||id.getGroupID()!=u8(value>>16)||id.getWaveID()!=u16(value))return 2;JAISoundID copy(id);if(u32(copy)!=value)return 3;JAISoundID joined(value>>24,(value>>16)&255,value&65535);if(u32(joined)!=value)return 4;id.set(0xab,0xcd,0xef01);if(u32(id)!=0xabcdef01)return 5;id.setSectionID(0x12);if(u32(id)!=0x12cdef01)return 6;id.setGroupID(0x34);if(u32(id)!=0x1234ef01)return 7;id.setWaveID(0x5678);if(u32(id)!=0x12345678)return 8;id.setAnonymous();if(!id.isAnonymous()||u32(id)!=0xffffffff)return 9;}
 JAISoundID truncated(0x102,0x100,0x10001);if(u32(truncated)!=STM_TITLE)return 10;
 std::puts("PASS native numeric sound IDs: original title constant, section/group/wave packing, constructors/copy, setter isolation, truncation and anonymous ID");
}