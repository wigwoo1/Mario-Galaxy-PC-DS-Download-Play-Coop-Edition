#include <nw4r/ut/CharStrmReader.h>
#include <cassert>
#include <cstdio>
using nw4r::ut::CharStrmReader;
int main() {
 static_assert(sizeof(wchar_t)==2,"Original text runtime requires UTF16 host wchar_t");
 const wchar_t wide[]={L'A',0x00e9,0x3042,0};
 CharStrmReader utf16(&CharStrmReader::ReadNextCharUTF16); utf16.Set(wide);
 for(unsigned i=0;i<4;++i){assert(utf16.Next()==wide[i]);assert(utf16.GetCurrentPos()==wide+i+1);}
 const char encoded[]={0x41,char(0xc3),char(0xa9),char(0xe3),char(0x81),char(0x82),0};
 CharStrmReader utf8(&CharStrmReader::ReadNextCharUTF8);utf8.Set(encoded);
 assert(utf8.Next()==0x41);assert(utf8.GetCurrentPos()==encoded+1);
 assert(utf8.Next()==0xe9);assert(utf8.GetCurrentPos()==encoded+3);
 assert(utf8.Next()==0x3042);assert(utf8.GetCurrentPos()==encoded+6);
 assert(utf8.Next()==0);
 const char sjis[]={char(0x82),char(0xa0),0x41,0};
 CharStrmReader japanese(&CharStrmReader::ReadNextCharSJIS);japanese.Set(sjis);
 assert(japanese.Next()==0x82a0);assert(japanese.GetCurrentPos()==sjis+2);assert(japanese.Next()==0x41);assert(japanese.Next()==0);
 const char single[]={char(0x80),char(0xff),0};
 CharStrmReader cp(&CharStrmReader::ReadNextCharCP1252);cp.Set(single);
 assert(cp.Next()==0x80);assert(cp.Next()==0xff);assert(cp.Next()==0);
 puts("PASS original character streams: Windows UTF16, UTF8, SJIS, CP1252 and cursor advancement");
}
