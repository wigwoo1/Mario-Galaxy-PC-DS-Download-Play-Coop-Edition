#include <Game/System/SysConfigFile.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>
extern "C" OSTime OSGetTime(){return 0x0102030405060708LL;}
int main(){SysConfigFile file;file.updateTimeAnnounced();file.setTimeSent(0x1122334455667788LL);file.setSentBytes(0x99aabbcc);u8 output[128]{};file.makeDataBinary(output,sizeof(output));assert(output[0]==1&&output[1]==1&&output[4]=='S'&&output[5]=='Y'&&output[6]=='S'&&output[7]=='C');const u8 expected[]={1,2,3,4,5,6,7,8,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc};assert(!std::memcmp(output+32,expected,sizeof(expected)));SysConfigFile restored;restored.loadFromDataBinary(output,52);assert(restored.getTimeAnnounced()==file.getTimeAnnounced()&&restored.getTimeSent()==file.getTimeSent()&&restored.getSentBytes()==file.getSentBytes());SysConfigChunk chunk;u8 payload[64]{};assert(chunk.serialize(payload,35)==-1);assert(chunk.serialize(payload,36)==36);for(u32 size=0;size<36;++size)assert(chunk.deserialize(payload,size)==3);payload[3]=255;assert(chunk.deserialize(payload,36)==3);std::puts("PASS system config: exact big-endian timestamps/count, file round trip, capacity and every truncated payload");}
