#include <JSystem/JAudio2/JASSeqReader.hpp>
#include <cassert>
#include <cstdio>
int main(){
 JASSeqReader r; assert(r.getOffset()==0 && r.mNumStacks==0);
 for(int i=0;i<8;++i) assert(!r.mStackPtrs[i] && !r.mLoopCounts[i]);
 u8 b[]={0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xff,0x7f,0x81,0,0xff,0xff,0xff,0x7f};
 r.init(b); assert(r.get24(0)==0x123456 && r.get16(1)==0x3456 && r.get32(1)==0x3456789a);
 assert(r.read16()==0x1234 && r.getOffset()==2); assert(r.read24()==0x56789a && r.getOffset()==5);
 for(u32 i=0;i<8;++i) assert(r.call(i)); auto c=r.getCur(); assert(!r.call(0) && r.getCur()==c);
 for(int i=0;i<8;++i) assert(r.ret()); assert(!r.ret());
 r.jump(u32(1)); assert(r.loopStart(2)); r.jump(u32(4)); assert(r.loopEnd() && r.getOffset()==1 && r.getLoopCount()==1);
 r.jump(u32(4)); assert(r.loopEnd() && r.getOffset()==4 && r.mNumStacks==0); assert(!r.loopEnd());
 r.jump(u32(8)); assert(r.readMidiValue()==127 && r.readMidiValue()==128 && r.readMidiValue()==0xfffffff);
 u8 bad[]={0x80,0x80,0x80,0x80,0}; r.init(bad); assert(r.readMidiValue()==0 && r.getOffset()==4);
 bool threw=false; try{NativeSequence::offset(b+1,b);}catch(const std::out_of_range&){threw=true;} assert(threw);
 auto base=reinterpret_cast<const u8*>(std::uintptr_t(0x100000000ull));
 assert(NativeSequence::offset(base,reinterpret_cast<const u8*>(std::uintptr_t(0x1ffffffffull)))==0xffffffffu);
 threw=false; try{NativeSequence::offset(base,reinterpret_cast<const u8*>(std::uintptr_t(0x200000000ull)));}catch(const std::out_of_range&){threw=true;} assert(threw);
 std::puts("PASS native sequence endian reads, pointer offsets, original call/loop/MIDI operations");
}
