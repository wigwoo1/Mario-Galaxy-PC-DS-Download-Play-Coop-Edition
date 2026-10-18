#include <windows.h>
#include <dbghelp.h>
#include <cstdio>
LONG CALLBACK traceCrash(EXCEPTION_POINTERS* e){
 if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
 std::fprintf(stderr,"FAULT RIP=%llx base=%llx address=%llx\n",(unsigned long long)e->ContextRecord->Rip,(unsigned long long)GetModuleHandleW(nullptr),(unsigned long long)e->ExceptionRecord->ExceptionInformation[1]);
 HANDLE process=GetCurrentProcess();
 SymSetOptions(SymGetOptions()|SYMOPT_LOAD_LINES|SYMOPT_UNDNAME|SYMOPT_DEFERRED_LOADS);
 if(!SymInitialize(process,nullptr,TRUE))return EXCEPTION_CONTINUE_SEARCH;
 wchar_t imagePath[MAX_PATH]{};GetModuleFileNameW(nullptr,imagePath,MAX_PATH);SymLoadModuleExW(process,nullptr,imagePath,nullptr,(DWORD64)GetModuleHandleW(nullptr),0,nullptr,0);
 CONTEXT context=*e->ContextRecord;STACKFRAME64 frame{};frame.AddrPC.Offset=context.Rip;frame.AddrPC.Mode=AddrModeFlat;frame.AddrStack.Offset=context.Rsp;frame.AddrStack.Mode=AddrModeFlat;frame.AddrFrame.Offset=context.Rbp;frame.AddrFrame.Mode=AddrModeFlat;
 for(unsigned depth=0;depth<24;++depth){
  if(!StackWalk64(IMAGE_FILE_MACHINE_AMD64,process,GetCurrentThread(),&frame,&context,nullptr,SymFunctionTableAccess64,SymGetModuleBase64,nullptr)||!frame.AddrPC.Offset)break;
  alignas(SYMBOL_INFO) char symbolStorage[sizeof(SYMBOL_INFO)+512]{};auto* symbol=reinterpret_cast<SYMBOL_INFO*>(symbolStorage);symbol->SizeOfStruct=sizeof(SYMBOL_INFO);symbol->MaxNameLen=511;DWORD64 displacement=0;
  IMAGEHLP_LINE64 line{};line.SizeOfStruct=sizeof(line);DWORD lineDisplacement=0;
  if(SymFromAddr(process,frame.AddrPC.Offset,&displacement,symbol))std::fprintf(stderr,"  #%u %s+0x%llx (%llx)",depth,symbol->Name,(unsigned long long)displacement,(unsigned long long)frame.AddrPC.Offset);
  else std::fprintf(stderr,"  #%u %llx",depth,(unsigned long long)frame.AddrPC.Offset);
  if(SymGetLineFromAddr64(process,frame.AddrPC.Offset,&lineDisplacement,&line))std::fprintf(stderr," %s:%lu",line.FileName,line.LineNumber);
  std::fputc('\n',stderr);
 }
 SymCleanup(process);return EXCEPTION_CONTINUE_SEARCH;
}
struct Install { Install(){AddVectoredExceptionHandler(1,traceCrash);} } install;
