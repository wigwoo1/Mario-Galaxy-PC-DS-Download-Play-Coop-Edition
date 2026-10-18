#include "Game/Util/StringUtil.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
int main(){
 char dst[40];
 MR::addFilePrefix(dst,40,"/Mario.arc","ObjectData");assert(!std::strcmp(dst,"ObjectData/Mario.arc"));
 MR::addFilePrefix(dst,5,"Mario.arc","ObjectData");assert(!std::strcmp(dst,"Obje"));
 assert(MR::removeExtensionString(dst,40,"ObjectData/Mario.arc")==dst+10);assert(!std::strcmp(dst,"ObjectData/Mario"));
 MR::removeExtensionString(dst,40,"Mario");assert(!std::strcmp(dst,"Mario"));
 MR::removeExtensionString(dst,40,"dir.ext/Mario");assert(!std::strcmp(dst,"dir.ext/Mario"));
 MR::removeExtensionString(dst,40,"dir/.hidden");assert(!std::strcmp(dst,"dir/.hidden"));
 MR::removeExtensionString(dst,40,"dir/Mario.multi.arc");assert(!std::strcmp(dst,"dir/Mario.multi"));
 char guarded[]={char(0x55),char(0x66),char(0x77)};
 assert(MR::convertUTF16ToASCII(guarded+1,L"Mario",1)==0&&guarded[0]==0x55&&guarded[1]==0&&guarded[2]==0x77);
 guarded[1]=0x66;assert(MR::convertUTF16ToASCII(guarded+1,L"Mario",0)==0&&guarded[1]==0x66);
 assert(MR::convertUTF16ToASCII(dst,L"Mario",40)==5&&!std::strcmp(dst,"Mario"));
 const wchar_t text[]={L'A',0xe9,0x100,L'B',0};
 assert(MR::convertUTF16ToASCII(dst,text,40)==2&&dst[0]=='A'&&u8(dst[1])==0xe9&&dst[2]==0);
 assert(MR::convertUTF16ToASCII(dst,L"Mario",3)==2&&!std::strcmp(dst,"Ma"));
 std::puts("PASS native runtime paths and UTF16: extension/null-separator cases, bounded output/canaries and host-endian code units");
}
