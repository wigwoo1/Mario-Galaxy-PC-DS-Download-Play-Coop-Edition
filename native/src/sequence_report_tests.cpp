#include <Native/SequenceReport.hpp>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cassert>
static char output[128];
void JASReport(const char* format,...){va_list arguments;va_start(arguments,format);std::vsnprintf(output,sizeof(output),format,arguments);va_end(arguments);}
int main(){const char text[]="native pointer";std::uintptr_t values[]={42,reinterpret_cast<std::uintptr_t>(text),255,7};unsigned char types[]={0,2,1,0};Native::reportSequence("%d %s %x %d",values,types);assert(!std::strcmp(output,"42 native pointer ff 7"));for(unsigned i=0;i<4;++i){values[i]=reinterpret_cast<std::uintptr_t>(text);types[i]=2;}Native::reportSequence("%s %s %s %s",values,types);assert(!std::strcmp(output,"native pointer native pointer native pointer native pointer"));std::puts("PASS typed sequence report: mixed integer/string arguments and full-width pointers");}
