#include <Game/RhythmLib/AudMeSeqReader.hpp>
#include <cassert>
#include <cstdio>
int main(){
 unsigned char bytes[32]={0x81,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};AudMeSeqReader r;r.init(bytes);
 assert(r.get16(1)==0x2345&&r.get24(0)==0x812345&&r.get24(1)==0x234567&&r.get32(1)==0x23456789);
 assert(r.read16()==0x8123&&r.read24()==0x456789&&r.getCur()==bytes+5);
 r.jump(0u);for(unsigned i=0;i<8;++i)assert(r.call(i+1));assert(!r.call(9)&&r.mNumStacks==8);
 for(unsigned i=0;i<8;++i)assert(r.ret());assert(!r.ret()&&r.getCur()==bytes);
 r.jump(3u);assert(r.loopStart(2));r.jump(7u);assert(r.loopEnd()&&r.getCur()==bytes+3&&r.getLoopCount()==1);r.jump(7u);assert(r.loopEnd()&&r.mNumStacks==0&&r.getCur()==bytes+7);assert(!r.loopEnd());
 r.init();assert(!r.mSeqBuff&&!r.mSeqCursor&&!r.mNumStacks);for(unsigned i=0;i<8;++i)assert(!r.mStackPtrs[i]&&!r.mLoopCounts[i]);
 puts("PASS ME sequence reader: BE unaligned scalars, exact 24-bit reads, cursor progression, call capacity, return and finite loop semantics");
}
