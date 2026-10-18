$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JKernel\JKRAramStream.cpp" -Raw
$init='OSInitMessageQueue(&sMessageQueue, sMessageBuffer, ARRAY_SIZEU(sMessageBuffer));'
$s=$s.Replace($init,'').Replace('    resume();','    '+$init+[Environment]::NewLine+'    resume();')
$s=$s.Replace('JKRAramPcs(0, (uintptr_t)buffer, destination, length, NULL);','if(!nativeUploadAram(destination,buffer,length)){writtenLength=0;break;}')
$s=$s.Replace('s32 readLength = command->mStream->read(buffer, length);','std::memset(buffer,0,length);'+[Environment]::NewLine+'            s32 readLength = command->mStream->read(buffer, length);').Replace('if (readLength == 0)','if (readLength <= 0)')
$s=$s.Replace('(OSMessage)writtenLength','reinterpret_cast<OSMessage>(std::uintptr_t(writtenLength))').Replace('== FALSE','== 0')
$s=$s.Replace('ARRAY_SIZEU(sMessageBuffer)','sizeof(sMessageBuffer)/sizeof(sMessageBuffer[0])')
$s=$s.Replace('new (JKRGetSystemHeap(), -4) JKRAramStreamCommand()','new (JKRGetSystemHeap(), -int(alignof(std::max_align_t))) JKRAramStreamCommand()')
$s=$s.Replace('JKRAramStream::~JKRAramStream() {' ,'JKRAramStream::~JKRAramStream() {'+[Environment]::NewLine+'    if(sAramStreamObject==this)sAramStreamObject=nullptr;')
$prefix=@"
#include <Native/HeapPlacement.hpp>
#include <cstring>
#include <cstdint>
#include "native_jkr_aram_port.hpp"
"@
Set-Content "$n\src\generated\AramStreamKernel.cpp" ($prefix+[Environment]::NewLine+$s)


