#include <cstdarg>
#include <cstdio>
extern "C" void OSReport(const char* format, ...) {
    if (!format) return;
    va_list args;
    va_start(args,format);
    std::vfprintf(stderr,format,args);
    va_end(args);
}
