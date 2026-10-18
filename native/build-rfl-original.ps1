param([switch]$DiagnosticBootTrace)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-sdk-mem-host.ps1')
$sdkMemIncludes=Join-Path $n 'build\sdk-mem-host\include'
$srcRoot=Join-Path $p 'Petari\src\RVLFaceLib'
$sdkRoot=Join-Path $p 'Petari\libs'
$outRoot=Join-Path $n 'build\rfl-original'
$headerRoot=Join-Path $outRoot 'include'
$compatRoot=Join-Path $outRoot 'c-compat'
$objectRoot=Join-Path $outRoot 'obj-c'
$sourceOverlay=Join-Path $outRoot 'src'
$zig=Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe'
if(-not (Test-Path $zig)){throw "Zig toolchain not found: $zig"}
if(-not $env:ZIG_GLOBAL_CACHE_DIR){$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'}
New-Item -ItemType Directory -Force $outRoot,$compatRoot,$objectRoot,$sourceOverlay|Out-Null

# Keep legacy header compatibility edits isolated to the build output. Original
# Petari source and SDK headers remain untouched.
Copy-Item (Join-Path $sdkRoot 'RVLFaceLib\include\*') $headerRoot -Recurse -Force
$nativeHeaderDir=Join-Path $headerRoot 'Native'
New-Item -ItemType Directory -Force -Path $nativeHeaderDir | Out-Null
Copy-Item -LiteralPath (Join-Path $n 'include\Native\RflArchive.h') -Destination (Join-Path $nativeHeaderDir 'RflArchive.h') -Force
$middleHeader=Join-Path $headerRoot 'RFL_MiddleDatabase.h'
$middleText=[IO.File]::ReadAllText($middleHeader)
if(-not $middleText.Contains('u8 dummy[0x18];')){throw 'Public middle database storage anchor missing.'}
# RFLiMiddleDB holds two native pointers and occupies40 bytes on this64bit host.
# Match its alignment too, so embedded instances do not corrupt adjacent fields.
$middleText=$middleText.Replace('u8 dummy[0x18];','union { u8 dummy[0x28]; void* nativeAlignment; };')
[IO.File]::WriteAllText($middleHeader,$middleText)
$internalMiddleHeader=Join-Path $headerRoot 'RFLi_MiddleDatabase.h'
$internalMiddleText=[IO.File]::ReadAllText($internalMiddleHeader)
$internalMiddleText=$internalMiddleText.Replace('} RFLiMiddleDB;','} RFLiMiddleDB;'+"`n"+'typedef char NativeRFLMiddleSizeCheck[(sizeof(RFLMiddleDB) >= sizeof(RFLiMiddleDB)) ? 1 : -1];'+"`n"+'typedef char NativeRFLMiddleAlignmentCheck[(__alignof__(RFLMiddleDB) >= __alignof__(RFLiMiddleDB)) ? 1 : -1];')
[IO.File]::WriteAllText($internalMiddleHeader,$internalMiddleText)
$databaseHeader=Join-Path $headerRoot 'RFLi_Database.h'
$databaseText=[IO.File]::ReadAllText($databaseHeader).Replace('<revolution/OS.h>','<revolution/os.h>')
[IO.File]::WriteAllText($databaseHeader,$databaseText)
$osCompatDir=Join-Path $headerRoot 'revolution'
New-Item -ItemType Directory -Force $osCompatDir|Out-Null
$osHeader=Join-Path $osCompatDir 'OS.h'
Copy-Item (Join-Path $sdkRoot 'RVL_SDK\include\revolution\os.h') $osHeader -Force
$osText=[IO.File]::ReadAllText($osHeader)
$osText=$osText.Replace('u32 __OSBusClock = 0x800000F8;','extern u32 __OSBusClock;').Replace('u32 __MEM2End = 0x80003128;','extern u32 __MEM2End;')
[IO.File]::WriteAllText($osHeader,$osText)
$internalFunctions=@(
 'RFLAddMiddleDBStoreData','RFLDrawShape','RFLGetExpression',
 'RFLGetMiddleDBStoredSize','RFLGetMiddleDBType','RFLiAddMiddleDBUserData',
 'RFLiCheckBirthday','RFLiCheckCtrlBufferCore','RFLiClearTableData',
 'RFLiGetExpressionNum','RFLiLoadControllerAsync','RFLInitRes',
 'RFLIsAvailableControllerData','RFLiTransformCoordinate',
 'RFLiUpdateMiddleDBAsync','RFLLoadControllerAsync','RFLSetCoordinate',
 'RFLSetMiddleDBHiddenMask','RFLSetMiddleDBRandomMask',
 'RFLUpdateMiddleDBAsync','RFLWaitAsync','RFLiSaveDatabaseAsync',
 'RFLiConvertInfo2Raw','RFLiConvertInfo2HRaw','RFLiConvertRaw2HRaw',
 'RFLiGetCharRawData','RFLiSetTemporaryID','RFLiIsValidName',
 'RFLiGetIsolation','RFLiGetHiddenHeader','RFLiDBIsLoaded',
 'RFLiCalculateCRC','RFLiCreateHeaderCRCAsync','RFLiCheckHeaderCRCAsync',
 'RFLiBootLoadAsync','RFLiAlloc','RFLiCriticalError'
)
$crossUnitFunctions=@('RFLiCheckCtrlBufferCore','RFLiSaveDatabaseAsync','RFLiConvertRaw2HRaw','RFLiGetHiddenHeader','RFLiDBIsLoaded','RFLiClearTableData')
foreach($name in $internalFunctions){
    foreach($header in Get-ChildItem $headerRoot -Filter '*.h' -Recurse){
        $text=[IO.File]::ReadAllText($header.FullName)
        $pattern="(?m)^(\s*)(?!static\b)([^\r\n;]*\b"+[regex]::Escape($name)+"\s*\()"
        $updated=[regex]::Replace($text,$pattern,'$1static $2')
        if($updated -cne $text){[IO.File]::WriteAllText($header.FullName,$updated)}
    }
}
foreach($name in $crossUnitFunctions){
    foreach($header in Get-ChildItem $headerRoot -Filter '*.h' -Recurse){
        $text=[IO.File]::ReadAllText($header.FullName)
        $pattern="(?m)^(\s*)static (\w+(?:\s*\*)?\s+"+[regex]::Escape($name)+"\s*\()"
        $updated=[regex]::Replace($text,$pattern,'$1$2')
        if($updated -cne $text){[IO.File]::WriteAllText($header.FullName,$updated)}
    }
}

$standardHeaders=@{
 cstdio='stdio.h';cstddef='stddef.h';cstdlib='stdlib.h';cstdarg='stdarg.h'
 cmath='math.h';cstring='string.h';cstdint='stdint.h'
}
foreach($name in $standardHeaders.Keys){
    [IO.File]::WriteAllText((Join-Path $compatRoot $name),"#include <$($standardHeaders[$name])>`n")
}
$platformDecls=@'
#include <revolution/os.h>
#include <revolution/wpad.h>
#include <revolution/mtx.h>
#include <revolution/gx.h>
#include <revolution/nand.h>
#define LENGTHOF(a) (sizeof(a) / sizeof((a)[0]))
s32 WPADReadFaceData(s32, void*, u32, u32, WPADCallback);
void C_MTXLookAt(Mtx, const Point3d*, const Vec*, const Point3d*);
u32 PSMTXInvXpose(const Mtx, Mtx);
void GXGetCullMode(GXCullMode*);
void NANDSetUserData(NANDCommandBlock*, void*);
void* NANDGetUserData(const NANDCommandBlock*);
s32 NANDSeek(NANDFileInfo*, s32, s32);
s32 NANDPrivateSafeOpenAsync(const char*, NANDFileInfo*, const u8, void*, const u32, NANDCallback, NANDCommandBlock*);
s32 NANDSafeCloseAsync(NANDFileInfo*, NANDCallback, NANDCommandBlock*);
s32 NANDGetLengthAsync(NANDFileInfo*, u32*, NANDCallback, NANDCommandBlock*);
'@
[IO.File]::WriteAllText((Join-Path $compatRoot 'rfl_platform_decls.h'),$platformDecls)

$sources=@(Get-ChildItem $srcRoot -Filter '*.c'|Sort-Object Name)
if($sources.Count -ne 14){throw "Expected 14 original RVLFaceLib units, found $($sources.Count)."}
$sourceCopies=@{}
foreach($source in $sources){
    $copy=Join-Path $sourceOverlay $source.Name
    $text=[IO.File]::ReadAllText($source.FullName)
    if($source.BaseName -eq 'RFL_Controller'){$text=$text.Replace('static BOOL RFLiCheckCtrlBufferCore','BOOL RFLiCheckCtrlBufferCore')}
    if($source.BaseName -eq 'RFL_Database'){
        $text=$text.Replace('static RFLErrcode RFLiSaveDatabaseAsync','RFLErrcode RFLiSaveDatabaseAsync')
        $text=$text.Replace('static void RFLiConvertRaw2HRaw','void RFLiConvertRaw2HRaw')
        $text=$text.Replace('static RFLiHiddenDB* RFLiGetHiddenHeader','RFLiHiddenDB* RFLiGetHiddenHeader')
        $text=$text.Replace('static BOOL RFLiDBIsLoaded','BOOL RFLiDBIsLoaded')
    }
    if($source.BaseName -eq 'RFL_Format'){$text=$text.Replace('static void RFLiClearTableData','void RFLiClearTableData')}
    if($source.BaseName -eq 'RFL_NANDLoader'){$text=& (Join-Path $n 'adapt-rfl-loader.ps1') -Source $text}
    if($DiagnosticBootTrace){
        $text="#include <stdio.h>`n"+$text
        if($source.BaseName-eq 'RFL_System'){
            $text=$text.Replace('        errcode = RFLiBootLoadAsync();','        fprintf(stderr,"RFL boot begin\n");'+"`n"+'        errcode = RFLiBootLoadAsync();'+"`n"+'        fprintf(stderr,"RFL boot returned err=%d working=%d last=%d reason=%ld\n",errcode,RFLiIsWorking(),RFLiGetManager()->lastErrCode,(long)RFLiGetManager()->lastReason);')
            $text=$text.Replace('    RFLWaitAsync();','    fprintf(stderr,"RFL exit wait begin\n");'+"`n"+'    RFLWaitAsync();'+"`n"+'    fprintf(stderr,"RFL exit wait returned\n");')
            $text=$text.Replace('    volatile RFLErrcode status;','    volatile RFLErrcode status; int previous=-999;')
            $text=$text.Replace('        status = RFLGetAsyncStatus();','        status = RFLGetAsyncStatus();'+"`n"+'        if(previous!=(int)status){fprintf(stderr,"RFL wait status=%d working=%d last=%d reason=%ld\n",status,RFLiIsWorking(),RFLiGetManager()?RFLiGetManager()->lastErrCode:-1,(long)RFLGetLastReason());previous=status;}')
        }
        if($source.BaseName-eq 'RFL_NANDAccess'){
            $text=[regex]::Replace($text,'(static void (?:open|close|read|write|readseek|writeseek|create|getlength)callback_\(s32 result, NANDCommandBlock\* block\)\s*\{)','$1'+"`n"+'    fprintf(stderr,"RFL NAND callback result=%ld block=%p\n",(long)result,(void*)block);')
        }
    }
    [IO.File]::WriteAllText($copy,$text)
    $sourceCopies[$source.BaseName]=$copy
}
$compiled=0
$previousCompilerErrorActionPreference=$ErrorActionPreference
# Zig/Clang writes diagnostics to stderr even on successful compiles. Older
# Windows PowerShell versions turn that stream into NativeCommandError records;
# keep it captured in the per-unit logs and use the process exit code below as
# the authoritative compile result.
$ErrorActionPreference='Continue'
foreach($source in $sources){
    $object=Join-Path $objectRoot ($source.BaseName+'.obj')
    $log=Join-Path $objectRoot ($source.BaseName+'.log')
    & $zig cc -std=gnu99 -DTRUE=1 -DFALSE=0 -fcommon -O2 -c -ferror-limit=24 -target x86_64-windows-gnu `
      "-I$sdkMemIncludes" "-I$compatRoot" "-I$headerRoot" "-I$(Join-Path $p 'Petari\include')" `
      "-I$(Join-Path $sdkRoot 'JSystem\include')" "-I$(Join-Path $sdkRoot 'RVL_SDK\include')" `
      "-I$(Join-Path $sdkRoot 'RVLFaceLib\include')" -include (Join-Path $compatRoot 'rfl_platform_decls.h') `
      $sourceCopies[$source.BaseName] -o $object 2>&1 | Out-File -FilePath $log -Encoding utf8
    if($LASTEXITCODE -ne 0){Get-Content $log -Tail 24;exit $LASTEXITCODE}
    $compiled++
}
$heapSource=Join-Path $n 'build\sdk-mem-host\src\mem_expHeap.c'
$heapObject=Join-Path $outRoot 'mem_expHeap.obj'
$heapLog=Join-Path $outRoot 'mem_expHeap.log'
& $zig cc -std=gnu99 -DTRUE=1 -DFALSE=0 -fcommon -O2 -c -ferror-limit=24 -target x86_64-windows-gnu `
  "-I$sdkMemIncludes" "-I$compatRoot" "-I$headerRoot" "-I$(Join-Path $p 'Petari\include')" `
  "-I$(Join-Path $sdkRoot 'JSystem\include')" "-I$(Join-Path $sdkRoot 'RVL_SDK\include')" `
  $heapSource -o $heapObject 2>&1 | Out-File -FilePath $heapLog -Encoding utf8
if($LASTEXITCODE -ne 0){Get-Content $heapLog -Tail 24;exit $LASTEXITCODE}
$commonSource=Join-Path $n 'build\sdk-mem-host\src\mem_heapCommon.c'
$commonObject=Join-Path $outRoot 'mem_heapCommon.obj'
$commonLog=Join-Path $outRoot 'mem_heapCommon.log'
& $zig cc -std=gnu99 -DTRUE=1 -DFALSE=0 -fcommon -O2 -c -ferror-limit=24 -target x86_64-windows-gnu `
  "-I$sdkMemIncludes" "-I$compatRoot" "-I$headerRoot" "-I$(Join-Path $p 'Petari\include')" `
  "-I$(Join-Path $sdkRoot 'JSystem\include')" "-I$(Join-Path $sdkRoot 'RVL_SDK\include')" `
  $commonSource -o $commonObject 2>&1 | Out-File -FilePath $commonLog -Encoding utf8
if($LASTEXITCODE -ne 0){Get-Content $commonLog -Tail 24;exit $LASTEXITCODE}
$listSource=Join-Path $n 'build\sdk-mem-host\src\mem_list.c'
$listObject=Join-Path $outRoot 'mem_list.obj'
$listLog=Join-Path $outRoot 'mem_list.log'
& $zig cc -std=gnu99 -DTRUE=1 -DFALSE=0 -fcommon -O2 -c -ferror-limit=24 -target x86_64-windows-gnu `
  "-I$sdkMemIncludes" "-I$compatRoot" "-I$headerRoot" "-I$(Join-Path $p 'Petari\include')" `
  "-I$(Join-Path $sdkRoot 'JSystem\include')" "-I$(Join-Path $sdkRoot 'RVL_SDK\include')" `
  $listSource -o $listObject 2>&1 | Out-File -FilePath $listLog -Encoding utf8
if($LASTEXITCODE -ne 0){Get-Content $listLog -Tail 24;exit $LASTEXITCODE}
$compatSource=Join-Path $n 'src\host_c_compat_port.c'
$compatObject=Join-Path $outRoot 'host_c_compat_port.obj'
$compatLog=Join-Path $outRoot 'host_c_compat_port.log'
& $zig cc -std=gnu99 -O2 -c -target x86_64-windows-gnu $compatSource -o $compatObject 2>&1 | Out-File -FilePath $compatLog -Encoding utf8
if($LASTEXITCODE -ne 0){Get-Content $compatLog -Tail 24;exit $LASTEXITCODE}
$ErrorActionPreference=$previousCompilerErrorActionPreference
Write-Output "Compiled $compiled/14 original RVLFaceLib units, three original MEM heap/list units, and one C-runtime ABI helper for the host link audit (compile-only; Wii services are not emulated)."
