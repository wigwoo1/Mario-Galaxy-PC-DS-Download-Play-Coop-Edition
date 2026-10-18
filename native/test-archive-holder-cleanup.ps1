param([ValidateSet('auto','msvc','zig')][string]$Compiler='auto',[ValidateSet('petari','native')][string]$Implementation='petari',[string]$ZigExePath)
$ErrorActionPreference='Stop'
$native=$PSScriptRoot
$project=Split-Path $native -Parent
$sourcePath=if($Implementation-eq 'petari'){"$project\Petari\src\Game\System\ArchiveHolder.cpp"}else{"$native\src\generated\ArchiveHolder.cpp"}
$source=Get-Content -LiteralPath $sourcePath -Raw
$destructorStart=$source.IndexOf('ArchiveHolderArchiveEntry::~ArchiveHolderArchiveEntry() {')
$destructorEnd=$source.IndexOf('ArchiveHolder::ArchiveHolder()', $destructorStart)
$removeStart=$source.IndexOf('void ArchiveHolder::removeIfIsEqualHeap(JKRHeap* pHeap) {')
$removeEnd=$source.IndexOf('ArchiveHolderArchiveEntry* ArchiveHolder::findEntry(', $removeStart)
if($destructorStart-lt 0-or $destructorEnd-lt 0-or $removeStart-lt 0-or $removeEnd-lt 0){throw 'Could not extract current ArchiveHolder cleanup methods.'}
$test=Get-Content -LiteralPath "$native\src\archive_holder_cleanup_tests.cpp" -Raw
$methods=$source.Substring($destructorStart,$destructorEnd-$destructorStart)+"`n"+$source.Substring($removeStart,$removeEnd-$removeStart)
$anchor='// ACTUAL_METHODS_INSERTION_POINT'
if(!$test.Contains($anchor)){throw 'Actual-method insertion anchor missing.'}
$test=$test.Replace($anchor,$methods)
if($Implementation-eq 'native'){$test="#define GALAXY_NATIVE_ARCHIVE_CLEANUP 1`n"+$test}
$output=Join-Path $native 'build\archive-holder-cleanup-test'
New-Item -ItemType Directory -Force -Path $output|Out-Null
$testSource=Join-Path $output 'archive_holder_cleanup_tests.cpp'
[IO.File]::WriteAllText($testSource,$test,[Text.UTF8Encoding]::new($false))
$exe=Join-Path $output 'archive-holder-cleanup-tests.exe'
$log=Join-Path $output 'build.log'
$vsWhere='C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vsInstall=$null
if(Test-Path -LiteralPath $vsWhere){$vsInstall=& $vsWhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath}
$vcvars=if($vsInstall){Join-Path $vsInstall 'VC\Auxiliary\Build\vcvars64.bat'}else{$null}
if($Compiler-eq 'auto'){$Compiler=if($vcvars-and(Test-Path -LiteralPath $vcvars)){'msvc'}else{'zig'}}
if($Compiler-eq 'msvc'){
 if(!$vcvars-or !(Test-Path -LiteralPath $vcvars)){throw 'MSVC was requested, but Visual Studio C++ Build Tools were not found.'}
 $command='"'+$vcvars+'" >nul && cl.exe /nologo /TP /std:c++17 /O2 /EHs-c- /GR- /GS- /Zl "'+$testSource+'" /link /NODEFAULTLIB /ENTRY:mainCRTStartup /SUBSYSTEM:CONSOLE kernel32.lib /OUT:"'+$exe+'"'
 $buildOutput=& cmd.exe /d /s /c $command 2>&1
 $buildExit=$LASTEXITCODE
 $buildOutput|Set-Content -LiteralPath $log
}else{
 $zig=if($ZigExePath){$ZigExePath}else{"$project\tools\zig-x86_64-windows-0.16.0\zig.exe"}
 if(!(Test-Path -LiteralPath $zig)){throw "Zig compiler not found: $zig"}
 $env:ZIG_GLOBAL_CACHE_DIR=Join-Path $output 'zig-cache'
 $env:ZIG_LOCAL_CACHE_DIR=Join-Path $output 'local-cache'
 $buildOutput=& $zig c++ -std=c++17 -O2 -fno-exceptions -fno-rtti -nostdlib -target x86_64-windows-gnu '-Wl,--entry,mainCRTStartup' $testSource -lkernel32 -o $exe 2>&1
 $buildExit=$LASTEXITCODE
 $buildOutput|Set-Content -LiteralPath $log
}
if($buildExit){Get-Content $log -Tail 30;exit 1}
& $exe
if($LASTEXITCODE){exit $LASTEXITCODE}
Write-Output 'PASS actual ArchiveHolder cleanup: matched entries unmount and destruct archives, then free name, archive, and entry from the owning heap exactly once.'
