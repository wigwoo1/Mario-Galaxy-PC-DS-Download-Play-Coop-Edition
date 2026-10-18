$ErrorActionPreference='Stop'
$native=$PSScriptRoot;$project=Split-Path $native -Parent
$output=Join-Path $env:TEMP 'galaxy-file-read-completion-test'
New-Item -ItemType Directory -Force -Path $output|Out-Null
$source=Get-Content -LiteralPath (Join-Path $native 'src\generated\FileHolder.cpp') -Raw
$methods=@()
foreach($signature in @('void FileHolderFileEntry::waitReadDone()', 'void FileHolderFileEntry::setContext(void* pContext, JKRHeap* pHeap)')){
 $start=$source.IndexOf($signature);if($start-lt 0){throw "Missing method: $signature"}
 $open=$source.IndexOf('{',$start);$depth=1;$end=$open+1
 while($depth){if($source[$end]-eq '{'){$depth++};if($source[$end]-eq '}'){$depth--};$end++}
 $methods+=$source.Substring($start,$end-$start)
}
$kernel=Join-Path $output 'completion-methods.cpp'
[IO.File]::WriteAllText($kernel,"#include <Game/System/FileHolder.hpp>`n#define FILE_HOLDER_STATE_DONE 2`n"+($methods-join "`n"))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $env:TEMP 'galaxy-prepresent-test-cache'
$exe=Join-Path $output 'completion-tests.exe';$log=Join-Path $output 'build.log'
& "$project\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$native\include-runtime" "-I$native\include" "-I$project\Petari\include" "-I$project\Petari\libs\RVL_SDK\include" "-I$project\Petari\libs\JSystem\include" -include revolution/types.h "$native\src\file_read_completion_tests.cpp" $kernel -o $exe *> $log
if($LASTEXITCODE){Get-Content $log -Tail 20;exit 1}
$process=Start-Process -FilePath $exe -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $output 'result.txt')
if(!$process.WaitForExit(10000)){Stop-Process -Id $process.Id;throw 'File completion test timed out.'}
Get-Content (Join-Path $output 'result.txt')
exit $process.ExitCode
