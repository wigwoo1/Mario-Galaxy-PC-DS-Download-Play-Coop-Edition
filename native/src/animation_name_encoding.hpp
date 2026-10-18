#pragma once
#include <windows.h>
// Explicit resource-name encoding bridge; no implicit platform code page.
inline std::string convertAnimationName(const std::string& text,UINT input,UINT output){
 assetCheck(text.size()<=4096&&text.find('\0')==std::string::npos,"Invalid animation name length/content");if(text.empty())return {};int count=MultiByteToWideChar(input,MB_ERR_INVALID_CHARS,text.data(),int(text.size()),nullptr,0);assetCheck(count>0,"Invalid encoded animation name");std::wstring wide(size_t(count),L'\0');assetCheck(MultiByteToWideChar(input,MB_ERR_INVALID_CHARS,text.data(),int(text.size()),wide.data(),count)==count,"Animation name decode failed");BOOL substituted=FALSE;DWORD flags=output==CP_UTF8?WC_ERR_INVALID_CHARS:WC_NO_BEST_FIT_CHARS;BOOL* used=output==CP_UTF8?nullptr:&substituted;int bytes=WideCharToMultiByte(output,flags,wide.data(),count,nullptr,0,nullptr,used);assetCheck(bytes>0&&!substituted,"Animation name not representable in target encoding");std::string result(size_t(bytes),'\0');substituted=FALSE;assetCheck(WideCharToMultiByte(output,flags,wide.data(),count,result.data(),bytes,nullptr,used)==bytes&&!substituted,"Animation name encode failed");return result;
}
inline std::string animationNameToGame(const std::string& name){return convertAnimationName(name,CP_UTF8,932);}
inline std::string animationNameFromGame(const std::string& name){return convertAnimationName(name,932,CP_UTF8);}
