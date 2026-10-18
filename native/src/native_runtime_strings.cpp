#include "Game/Util/StringUtil.hpp"
#include <cstdio>
#include <cstring>
namespace MR {
void addFilePrefix(char* dst,u32 size,const char* child,const char* parent){
 if(!dst||!size)return;
 if(!child||!parent){dst[0]=0;return;}
 if(*child=='/')++child;
 std::snprintf(dst,size,"%s/%s",parent,child);
}
char* removeExtensionString(char* dst,u32 size,const char* path){
 if(!dst||!size)return nullptr;
 if(!path){dst[0]=0;return nullptr;}
 std::snprintf(dst,size,"%s",path);
 char* slash=std::strrchr(dst,'/');
 char* dot=std::strrchr(dst,'.');
 char* basename=slash?slash+1:dst;
 if(dot&&dot>basename)*dot=0;
 return slash;
}
int convertUTF16ToASCII(char* dst,const wchar_t* src,s32 capacity){
 if(!dst||capacity<=0)return 0;
 int count=0;
 if(src)while(count<capacity-1&&src[count]!=0&&static_cast<unsigned>(src[count])<=0xff){
  dst[count]=static_cast<char>(src[count]);++count;
 }
 dst[count]=0;return count;
}
}
