param([switch]$Original)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-sdk-mem-host.ps1')
$out=Join-Path $n 'build\sdk-mem-host';$zig=Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$includes=@()
if(-not $Original){$includes+=('-I'+(Join-Path $out 'include'))}
foreach($rel in @('native\build\rfl-original\c-compat','native\build\rfl-original\include','Petari\libs\RVL_SDK\include')){$includes+=('-I'+(Join-Path $p $rel))}
$objects=@()
foreach($unit in @('mem_expHeap','mem_heapCommon','mem_list')){
 $source=if($Original){Join-Path $p ('Petari\src\RVL_SDK\mem\'+$unit+'.c')}else{Join-Path $out ('src\'+$unit+'.c')}
 $obj=Join-Path $out ($unit+'-test.obj');$objects+=$obj
 & $zig cc -std=gnu99 -DTRUE=1 -DFALSE=0 -fcommon -O2 -c -target x86_64-windows-gnu @includes $source -o $obj *> (Join-Path $out ($unit+'-test.log'))
 if($LASTEXITCODE){Get-Content (Join-Path $out ($unit+'-test.log')) -Tail 16;exit $LASTEXITCODE}
}
$exe=Join-Path $out 'sdk-mem-tests.exe'
& $zig cc -std=gnu99 -DTRUE=1 -DFALSE=0 -O2 -UNDEBUG -target x86_64-windows-gnu @includes (Join-Path $n 'src\sdk_mem_heap_tests.c') @objects -o $exe *> (Join-Path $out 'tests-build.log')
if($LASTEXITCODE){Get-Content (Join-Path $out 'tests-build.log') -Tail 16;exit $LASTEXITCODE}
& $exe
exit $LASTEXITCODE
