#include <JSystem/JAudio2/JASSeqReader.hpp>
#include <JSystem/JAudio2/JASTrackPort.hpp>
#include <cassert>
#include <cstdio>
int main(){u8 bytes[]={0x12,0x34,0x56,0x78,0x81,0x00,0x7f,0};JASSeqReader reader;reader.init(bytes);
 JASTrackPort ports;ports.init();for(unsigned i=0;i<16;++i){assert(!ports.checkImport(i)&&!ports.checkExport(i));ports.writeImport(i,u16(0x100+i));ports.writeExport(i,u16(0x200+i));assert(ports.checkImport(i)&&ports.checkExport(i));assert(ports.readImport(i)==0x200+i&&!ports.checkImport(i)&&ports.checkExport(i));assert(ports.readExport(i)==0x200+i&&!ports.checkExport(i));}ports.writeImport(15,123);ports.init();assert(ports.readImport(15)==0&&!ports.checkImport(15));
 assert(reader.get16(1)==0x3456&&reader.get24(0)==0x123456&&reader.get32(0)==0x12345678);
 assert(reader.read24()==0x123456&&reader.getOffset()==3);reader.jump(0u);assert(reader.read16()==0x1234&&reader.getOffset()==2);
 reader.jump(4u);assert(reader.readMidiValue()==128&&reader.readMidiValue()==127);
 reader.jump(0u);assert(reader.call(3)&&reader.getOffset()==3&&reader.ret()&&reader.getOffset()==0&&!reader.ret());
 assert(reader.loopStart(2));reader.jump(3u);assert(reader.loopEnd()&&reader.getOffset()==0);reader.jump(3u);assert(reader.loopEnd()&&reader.getOffset()==3);
 for(int i=0;i<8;++i)assert(reader.call(0));assert(!reader.call(0));for(int i=0;i<8;++i)assert(reader.ret());
 std::puts("PASS native sequence reader endian/unaligned reads, cursor, MIDI values, calls, loops and stack limits");}
