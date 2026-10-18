#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#if defined(_WIN32)
#include <windows.h>
#endif
extern "C" void OSPanic(const char* file,int line,const char* format,...){
 char detail[1024];
 va_list arguments;va_start(arguments,format);
 if(format){va_list debugArguments;va_copy(debugArguments,arguments);std::vsnprintf(detail,sizeof(detail),format,debugArguments);va_end(debugArguments);}else{std::snprintf(detail,sizeof(detail),"<no message>");}
 va_end(arguments);
 std::fprintf(stderr,"Native runtime panic at %s:%d: %s\n",file?file:"<unknown>",line,detail);
 std::fflush(stderr);
#if defined(_WIN32)
 char debugMessage[1280];
 std::snprintf(debugMessage,sizeof(debugMessage),"Native runtime panic at %s:%d: %s\n",file?file:"<unknown>",line,detail);
 OutputDebugStringA(debugMessage);
#endif
 std::abort();
}
// SDK OS.c: OSRegisterVersion reports the supplied identifier followed by a newline.
extern "C" void OSRegisterVersion(const char* identifier){std::fprintf(stdout,"%s\n",identifier?identifier:"<null>");}
