#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <JSystem/JUtility/JUTException.hpp>
namespace {void report(FILE* stream,const char* format,va_list arguments){if(format)std::vfprintf(stream,format,arguments);std::fflush(stream);}}
extern "C" void JUTWarningConsole(const char* message){if(message)std::fputs(message,stderr);std::fflush(stderr);}
extern "C" void JUTWarningConsole_f(const char* format,...){va_list arguments;va_start(arguments,format);report(stderr,format,arguments);va_end(arguments);}
void JUTException::panic_f_va(const char* file,int line,const char* format,va_list arguments){std::fprintf(stderr,"Native JSystem panic at %s:%d: ",file?file:"<unknown>",line);report(stderr,format,arguments);std::fputc('\n',stderr);std::fflush(stderr);std::abort();}
void JUTException::panic_f(const char* file,int line,const char* format,...){va_list arguments;va_start(arguments,format);panic_f_va(file,line,format,arguments);va_end(arguments);}
