$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-sdk-mem-host.ps1')
$out=Join-Path $n 'build\sdk-mem-host';$zig=Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$includes=@('-I'+(Join-Path $out 'include'))
foreach($rel in @('native\build\rfl-original\c-compat','native\build\rfl-original\include','Petari\libs\RVL_SDK\include')){$includes+=('-I'+(Join-Path $p $rel))}
foreach($unit in @('mem_expHeap','mem_heapCommon','mem_list')){
 $log=Join-Path $out ($unit+'.log')
 & $zig cc -std=gnu99 -DTRUE=1 -DFALSE=0 -fcommon -O2 -c -target x86_64-windows-gnu @includes (Join-Path $out ('src\'+$unit+'.c')) -o (Join-Path $out ($unit+'.obj')) *> $log
 if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 16;exit $LASTEXITCODE}
}
Write-Output 'PASS three original SDK heap/list providers compiled with full host pointers and alignment.'
