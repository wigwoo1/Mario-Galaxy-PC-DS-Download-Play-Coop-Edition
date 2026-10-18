#include <Game/Screen/ReplaceTagProcessor.hpp>
#include <Game/Screen/MessageTagSkipTagProcessor.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Unrelated game services are isolated; the replacement engine and tag accessors
// below use the same Windows tag decoder as the full native runtime.
namespace MR {
bool isMessageEditorNextTag(const wchar_t*) { return false; }
void copyMemory(void* dst,const void* src,u32 bytes) { std::memcpy(dst,src,bytes); }
void copyString(wchar_t* dst,const wchar_t* src,u32 count) { if(!count)return; std::wcsncpy(dst,src,count-1);dst[count-1]=0; }
bool isPlayerLuigi() { return false; }
const wchar_t* getGameMessageDirect(const char*) { return L"Mario"; }
int getStringLengthWithMessageTag(const wchar_t* text) { return int(std::wcslen(text)); }
void makeRaceCurrentTimeString(wchar_t* dst) { std::wcscpy(dst,L"00:00:00"); }
void makeRaceBestTimeString(wchar_t* dst,int) { std::wcscpy(dst,L"00:00:00"); }
}
std::wstring tag(unsigned group,unsigned code,unsigned index) {
 auto be16=[](unsigned x){return wchar_t(((x&255)<<8)|((x>>8)&255));};
 return {wchar_t(0x1a),wchar_t((group<<8)|14),be16(code),0,0,be16(index>>16),be16(index)};
}
void checkNumber(unsigned code,const wchar_t* expected) {
 auto format=L"Before "+tag(6,code,2)+L" / "+tag(6,code,0)+L" after";
 wchar_t result[512]{};
 ReplaceTagFunction::ReplaceArgs(result,512,format.c_str(),7,99,42);
 const std::wstring want=std::wstring(L"Before ")+expected+L" / "+(code==5?L"07":code==6?L"007":code==7?L"0007":code==8?L"00007":code==9?L"000007":L"7")+L" after";
 if(result!=want){std::fprintf(stderr,"number code=%u actual:",code);for(const auto c:std::wstring(result))std::fprintf(stderr," %04x",unsigned(c));std::fprintf(stderr," expected:");for(const auto c:want)std::fprintf(stderr," %04x",unsigned(c));std::fprintf(stderr,"\n");}
 assert(result==want);
}
void checkIndependent(const wchar_t* format,...) {
 va_list args;va_start(args,format);
 wchar_t first[512]{},second[512]{};
 ReplaceTagProcessor::ReplaceArgs(first,format,args);
 ReplaceTagProcessor::ReplaceArgs(second,format,args);
 assert(!std::wcscmp(first,L"Luigi Mario Luigi"));
 assert(!std::wcscmp(first,second));
 assert(!std::wcscmp(va_arg(args,const wchar_t*),L"Mario"));
 va_end(args);
}
void checkRetail(const char* image) {
 WiiAssets disc(std::filesystem::u8path(image));
 Rarc archive(disc.readFile("/UsEnglish/MessageData/Message.arc"));
 NativeBmgText messages(archive.readFile("/message.bmg"));
 auto ids=archive.readFile("/messageid.tbl");StageTable table{BinaryView(ids)};
 const int nameField=table.find("MessageId"),indexField=table.find("Index");
 assert(nameField>=0&&indexField>=0);
 bool checked=false;
 for(size_t row=0;row<table.rows;++row){
  auto name=table.read(row,size_t(nameField));
  if(!std::holds_alternative<std::string>(name)||std::get<std::string>(name)!="System_Time000")continue;
  auto index=table.read(row,size_t(indexField));assert(std::holds_alternative<uint32_t>(index));
  NativeTaggedMessage text(messages.tokens(std::get<uint32_t>(index)));
  wchar_t rendered[512]{};
  {NativeMessageTagScope scope(text);ReplaceTagFunction::ReplaceArgs(rendered,512,text.data(),0,59,59,99);}
  std::string visible,digits;
  for(size_t i=0;rendered[i];){
   if(rendered[i]==0x1a){MessageEditorMessageTag tag(rendered+i+1);assert(tag.getGroup()!=6);i+=tag.getSkipLength()+1;continue;}
   if(rendered[i]<128)visible.push_back(char(rendered[i]));
   if(rendered[i]>=L'0'&&rendered[i]<=L'9')digits.push_back(char(rendered[i]));
   ++i;
  }
  assert(digits=="595999");
  std::printf("PASS actual WBFS System_Time000 numeric formatting: %s\n",visible.c_str());
  checked=true;break;
 }
 assert(checked);
}
int main(int argc,char** argv) {
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 static_assert(sizeof(wchar_t)==2,"Native message tags require UTF-16");
 checkNumber(0,L"42");checkNumber(5,L"42");checkNumber(6,L"042");
 checkNumber(7,L"0042");checkNumber(8,L"00042");checkNumber(9,L"000042");
 wchar_t output[512]{};auto number=tag(6,0,0);
 ReplaceTagFunction::ReplaceArgs(output,512,number.c_str(),99);assert(!std::wcscmp(output,L"99"));
 ReplaceTagFunction::ReplaceArgs(output,512,number.c_str(),-12);assert(!std::wcscmp(output,L"-12"));
 auto strings=tag(7,0,1)+L" "+tag(7,0,0)+L" "+tag(7,0,1);
 checkIndependent(strings.c_str(),L"Mario",L"Luigi");
 auto unhandled=tag(4,11,0);ReplaceTagFunction::ReplaceArgs(output,512,unhandled.c_str());
 assert(!std::memcmp(output,unhandled.c_str(),(unhandled.size()+1)*sizeof(wchar_t)));
 if(argc==2)checkRetail(argv[1]);
 std::puts("PASS original message arguments: numeric argument99, signed numbers, widths2..6, argument indexing, repeated string tags, caller va_list preserved, unknown tag retained");
}
