#include <JSystem/JAudio2/JASReport.hpp>
#include <cstdarg>
#include <cstdio>
#include <mutex>
namespace { std::mutex reportMutex; }
// Native diagnostics remain available before original audio initialization.
// Limit individual messages without truncating pointer-valued variadic arguments.
void JASReport(const char* format, ...) {
 if (!format) return;
 char message[1024];
 va_list args; va_start(args,format);
 const int result=std::vsnprintf(message,sizeof(message),format,args);
 va_end(args);
 if(result<0) return;
 const std::lock_guard<std::mutex> lock(reportMutex);
 std::fputs("[JAudio] ",stderr);
 std::fputs(message,stderr);
 std::fputc('\n',stderr);
}
