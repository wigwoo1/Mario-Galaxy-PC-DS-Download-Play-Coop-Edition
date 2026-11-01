param([switch]$DiagnosticPool)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$n\build\jai-stream-manager-cache"
$out="$n\build\actor-runtime-jai-stream-manager"
New-Item -ItemType Directory -Force -Path $out|Out-Null
$source="$p\Petari\src\JSystem\JAudio2\JAIStreamMgr.cpp"
$adapted="$out\JAIStreamMgrHost.cpp"
$original=[IO.File]::ReadAllText($source)
$old='bool result = mStreamAramMgr->deleteStreamAram((u32)aramAddr);'
$new='bool result = reinterpret_cast<uintptr_t>(aramAddr) <= UINT32_MAX && mStreamAramMgr->deleteStreamAram(static_cast<u32>(reinterpret_cast<uintptr_t>(aramAddr)));'
if(-not $original.Contains($old)){throw 'Expected original JAIStreamMgr ARAM address conversion was not found'}
$text=$original.Replace($old,$new)
$success='    return false;(\r?\n\}\r?\n\r?\nvoid JAIStreamMgr::freeDeadStream_\(\))'
if([regex]::Matches($text,$success).Count-ne 1){throw 'Original stream start success return anchor changed'}
$text=[regex]::Replace($text,$success,'    return true;$1')
if($DiagnosticPool){
 $trace=@'
#include <cstdio>
extern "C" void galaxyNativeBootTrace(const char*);
static void traceStreamPool(const char* event,JAIStreamMgr* manager,u32 id,JAIStream* stream){
    char line[320];auto& pool=JAIStream::memPool_;
    std::snprintf(line,sizeof(line),"Stream pool event=%s manager=%p request=%08X stream=%p free=%u total=%u active=%d",event,static_cast<void*>(manager),unsigned(id),static_cast<void*>(stream),unsigned(pool.getFreeMemCount()),unsigned(pool.getTotalMemCount()),manager->getNumActiveStreams());
    galaxyNativeBootTrace(line);
    for(auto* link=manager->getStreamList()->getFirst();link;link=link->getNext()){
        auto* current=link->getObject();
        std::snprintf(line,sizeof(line),"Stream member object=%p id=%08X state=%d prepare=%d stopped=%u started=%u entry=%d",static_cast<void*>(current),unsigned(u32(current->getID())),int(current->mStatus.getState()),current->mPrepareState,unsigned(current->mIsStreamStopped),unsigned(current->mIsStreamStarted),current->mStreamFileEntry);
        galaxyNativeBootTrace(line);
    }
}
'@
 foreach($spec in @(
  @('    JAIStream* stream = newStream_();',('    traceStreamPool("request",this,u32(id),nullptr);'+"`n"+'    JAIStream* stream = newStream_();'+"`n"+'    if(!stream)traceStreamPool("exhausted",this,u32(id),nullptr);')),
  @('    stream->JAIStreamMgr_startID_(id, streamFileEntry, posPtr, mAudience, category);',('    stream->JAIStreamMgr_startID_(id, streamFileEntry, posPtr, mAudience, category);'+"`n"+'    traceStreamPool("started",this,u32(id),stream);')),
  @('        if (stream->isDead()) {',('        if (stream->isDead()) {'+"`n"+'            traceStreamPool("release",this,u32(stream->getID()),stream);')),
  @('            delete stream;',('            delete stream;'+"`n"+'            traceStreamPool("released",this,0,nullptr);'))
 )){
  if([regex]::Matches($text,[regex]::Escape($spec[0])).Count-ne 1){throw 'Stream diagnostic anchor changed'}
  $text=$text.Replace($spec[0],$spec[1])
 }
 $anchor='#include "JSystem/JAudio2/JAIStreamDataMgr.hpp"'
 if(!$text.Contains($anchor)){throw 'Stream diagnostic include anchor changed'}
 $text=$text.Replace($anchor,$anchor+"`n"+$trace)
}
[IO.File]::WriteAllText($adapted,$text,[Text.UTF8Encoding]::new($false))
$compiler="$p\tools\zig-x86_64-windows-0.16.0\zig.exe"
$common=@('-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER','-std=c++17','-O2','-c','-target','x86_64-windows-gnu',"-I$n\include-runtime","-I$n\include","-I$p\Petari\include","-I$p\Petari\libs\JSystem\include","-I$p\Petari\libs\RVL_SDK\include","-I$p\Petari\libs\RVLFaceLib\include","-I$p\Petari\libs\nw4r\include",'-include',"$n\include-runtime\native_no_weak.h",'-include','revolution/types.h','-include',"$n\include-runtime\nw4r\ut\TagProcessorBase.h",'-include',"$n\include-runtime\nw4r\ut\Color.h")
& $compiler c++ @common $adapted -o "$out\JAIStreamMgr.obj" *> "$out\compile.log"
if($LASTEXITCODE){Get-Content "$out\compile.log" -Tail 24;exit 1}
Write-Output 'PASS original JAIStreamMgr compiled with a checked Wii ARAM address conversion'
