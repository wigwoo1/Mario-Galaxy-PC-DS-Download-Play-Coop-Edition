#include <JSystem/JAudio2/JAUStreamFileTable.hpp>
#include <revolution/dvd.h>
#include <cassert>
#include <cstring>
#include <cstdio>
extern "C" s32 DVDConvertPathToEntrynum(const char* path){return !std::strcmp(path,"a.ast")?17:-1;}
int main(){const u8 bytes[]={'b','s','f','t',0,0,0,2,0,0,0,16,0,0,0,22,'a','.','a','s','t',0,'b','.','a','s','t',0};JAUStreamFileTable table;assert(!table.isValid()&&table.getNumFiles()==0&&!table.getFilePath(0));table.init(bytes);assert(table.isValid()&&table.getNumFiles()==2);assert(!std::strcmp(table.getFilePath(0),"a.ast")&&!std::strcmp(table.getFilePath(1),"b.ast"));assert(!table.getFilePath(-1)&&!table.getFilePath(2));JAUStreamDataMgr_StreamFileTable mgr;mgr.init(bytes);assert(mgr.getStreamFileEntry(JAISoundID(0x02000000))==17);assert(mgr.getStreamFileEntry(JAISoundID(0x02000001))==-1);assert(mgr.getStreamFileEntry(JAISoundID(0x0200ffff))==-1);table.init(nullptr);assert(!table.isValid());std::puts("PASS stream file table: Wii BE count/offsets, borrowed paths, null and out-of-range indices");}
