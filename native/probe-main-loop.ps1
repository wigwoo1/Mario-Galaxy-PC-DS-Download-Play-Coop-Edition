$ErrorActionPreference='Continue'
$PSNativeCommandUseErrorActionPreference=$false
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$out="$n\build\actor-runtime-probe"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$unit='MainLoopFramework';$source="$n\src\generated\MainLoopFramework.cpp"
$s=Get-Content -Raw "$p\Petari\src\Game\System\MainLoopFramework.cpp"
$s=& "$n\adapt-clear-depth-texture.ps1" -SourceText $s
$s=$s.Replace('#include <runtime.h>','').Replace('__cvt_dbl_usll(OS_BUS_CLOCK / 4 * 0.5)','static_cast<u64>(OS_BUS_CLOCK / 4 * 0.5)').Replace('VISetBlack(TRUE)','VISetBlack(1)')
$s=$s.Replace('u32 msg;','OSMessage nativeMessage;'+"`n"+'            u32 msg;').Replace('(OSMessage*)&msg','&nativeMessage').Replace('                    msg = dummy;','                    nativeMessage = nullptr;').Replace('            } while ((s32)(msg - nextCount) < 0);','                msg = static_cast<u32>(reinterpret_cast<std::uintptr_t>(nativeMessage));'+"`n"+'            } while ((s32)(msg - nextCount) < 0);')
if(!$s.Contains('GX_WRITE_U8(0x61);') -or !$s.Contains('GX_WRITE_U32(0x5800000f);') -or !$s.Contains('DrawSyncManager::prepareReset();')){throw 'Original abort FIFO/recovery anchors changed'}
$s=$s.Replace('#include <revolution/gx/GXRegs.h>','#include <revolution/gd/GDBase.h>').Replace('GX_WRITE_U8(0x61);','GDWrite_u8(0x61);').Replace('GX_WRITE_U32(0x5800000f);','GDWrite_u32(0x5800000f);')
$s=[regex]::Replace($s,'GDWrite_u8\(0x61\);\s*GDWrite_u32\(0x5800000f\);','Native::appendGxAbortResetCommand();',1)
$s=$s.Replace('DrawSyncManager::prepareReset();','DrawSyncManager::prepareReset();' + [Environment]::NewLine + '        Native::markGxAbortRecoveryReady();')
if($s -match 'GX_WRITE_|GXWGFifo'){throw 'Unadapted hardware FIFO access remains'}
$s=[regex]::Replace($s,'(?<![\w.])([01])f\b','$1.0f')
Set-Content $source ('#include <cstdarg>'+"`n"+'#include <cstdint>'+"`n"+'#include <Native/CommandSubmission.hpp>'+"`n"+'using __va_list = va_list;'+"`n"+$s)
$line=Get-Content "$n\probe-actor-runtime.ps1"|Where-Object {$_ -like '& *zig.exe*'}
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 40;exit 1}



