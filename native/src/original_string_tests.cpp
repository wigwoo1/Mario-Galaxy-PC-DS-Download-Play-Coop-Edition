#include <cassert>
#include <cstring>
#include <cwchar>
namespace MR {int strcasecmp(const char*,const char*);bool isEqualStringCase(const char*,const char*);void copyString(char*,const char*,unsigned);void copyString(wchar_t*,const wchar_t*,unsigned);}
int main(){assert(MR::strcasecmp("Mario.arc","MARIO.ARC")==0);assert(MR::strcasecmp("a","b")<0);assert(MR::isEqualStringCase("/LayoutData/Font.arc","/layoutdata/font.ARC"));assert(!MR::isEqualStringCase("font","fonts"));char narrow[5]={'x','x','x','x','z'};MR::copyString(narrow,"a",4);assert(narrow[0]=='a' && narrow[1]==0 && narrow[3]==0 && narrow[4]=='z');MR::copyString(narrow,"abcde",4);assert(!std::memcmp(narrow,"abcdz",5));wchar_t wide[4]={L'x',L'x',L'x',L'z'};MR::copyString(wide,L"\u3042",3);assert(wide[0]==0x3042 && wide[1]==0 && wide[2]==0 && wide[3]==L'z');}
