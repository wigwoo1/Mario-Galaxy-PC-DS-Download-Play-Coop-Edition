#include <Native/GameLanguage.hpp>
#include <Game/System/Language.hpp>
#include <revolution/sc.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <stdexcept>
int main() {
 assert(Native::gameLanguage()==Native::GameLanguage::English);
 struct Case {Native::GameLanguage language; u32 id; const char* prefix;};
 const Case cases[]={{Native::GameLanguage::English,0x21,"UsEnglish"},
 {Native::GameLanguage::French,0x23,"UsFrench"},
 {Native::GameLanguage::Spanish,0x24,"UsSpanish"}};
 for(const auto& c:cases) {
  Native::setGameLanguage(c.language);
  assert(SCGetLanguage()==static_cast<u8>(c.language));
  assert(MR::getDecidedLanguageFromIPL()==c.id);
  bool found=false;
  // Original resource prefix table must contain the expected names.
  for(u32 i=0;i<MR::getLanguageNum();++i)
   found|=std::strcmp(MR::getLanguagePrefixByIndex(i),c.prefix)==0;
  assert(found);
 }
 for(unsigned value=0;value<256;++value) {
  if(value==1||value==3||value==4)continue;
  const auto before=Native::gameLanguage();
  bool rejected=false;
  try {Native::setGameLanguage(static_cast<Native::GameLanguage>(value));}
  catch(const std::invalid_argument&) {rejected=true;}
  assert(rejected&&Native::gameLanguage()==before);
 }
 Native::setGameLanguage(Native::GameLanguage::English);
 std::puts("PASS original RMGE resource language selection: English/French/Spanish; unsupported settings reject without mutation");
}
