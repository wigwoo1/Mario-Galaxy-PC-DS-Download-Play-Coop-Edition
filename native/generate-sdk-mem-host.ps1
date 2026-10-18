$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\sdk-mem-host';$headers=Join-Path $out 'include\revolution\mem'
New-Item -ItemType Directory -Force -Path $headers,(Join-Path $out 'src') | Out-Null
function Replace-Required([string]$text,[string]$old,[string]$new){
 if(-not $text.Contains($old)){throw "SDK heap transformation anchor missing: $old"}
 return $text.Replace($old,$new)
}
$h=Get-Content -LiteralPath (Join-Path $p 'Petari\libs\RVL_SDK\include\revolution\mem\heapCommon.h') -Raw
$h=Replace-Required $h 'typedef u32 UIntPtr;' "#include <stdint.h>`n typedef uintptr_t UIntPtr;"
$h=Replace-Required $h 'RoundUp(GetUIntPtr(ptr), (alignment))' 'RoundUp(GetUIntPtr(ptr), (UIntPtr)(alignment))'
$h=Replace-Required $h 'RoundDown(GetUIntPtr(ptr), (alignment))' 'RoundDown(GetUIntPtr(ptr), (UIntPtr)(alignment))'
$h=Replace-Required $h 'return wa - wb;' 'return (GetUIntPtr(wa) > GetUIntPtr(wb)) - (GetUIntPtr(wa) < GetUIntPtr(wb));'
[IO.File]::WriteAllText((Join-Path $headers 'heapCommon.h'),$h)
foreach($unit in @('mem_expHeap','mem_heapCommon','mem_list')){
 $s=Get-Content -LiteralPath (Join-Path $p ('Petari\src\RVL_SDK\mem\'+$unit+'.c')) -Raw
 if($unit-eq 'mem_list'){
  $s=Replace-Required $s '(((u32)(obj))+(parent_list)->offs)' '((u8*)(obj)+(parent_list)->offs)'
 }
 if($unit-eq 'mem_expHeap'){
  $s=Replace-Required $s 'pMBlkHd->blockSize >= size + offset' 'pMBlkHd->blockSize >= size && pMBlkHd->blockSize - size >= offset'
  $s=Replace-Required $s 'RoundDownPtr(AddU32ToPtr(startAddress, size), 4)' 'RoundDownPtr(AddU32ToPtr(startAddress, size), __alignof__(MEMiHeapHead))'
  $s=Replace-Required $s 'RoundUpPtr(startAddress, 4)' 'RoundUpPtr(startAddress, __alignof__(MEMiHeapHead))'
  $s=Replace-Required $s 'size = RoundUp(size, 4);' "if (size > UINT32_MAX - (__alignof__(MEMiExpHeapMBlockHead) - 1)) return NULL;`n    size = RoundUp(size, __alignof__(MEMiExpHeapMBlockHead));`n    if (alignment == 0 || alignment == INT32_MIN) return NULL;`n    if ((unsigned)__abs(alignment) & ((unsigned)__abs(alignment) - 1)) return NULL;`n    if (alignment > 0 && alignment < __alignof__(MEMiExpHeapMBlockHead)) alignment = __alignof__(MEMiExpHeapMBlockHead);`n    if (alignment < 0 && -alignment < __alignof__(MEMiExpHeapMBlockHead)) alignment = -__alignof__(MEMiExpHeapMBlockHead);"
  $s=Replace-Required $s 'alignment = __abs(alignment);' "if (alignment == 0 || alignment == INT32_MIN) return 0;`n    alignment = __abs(alignment);`n    if ((unsigned)alignment & ((unsigned)alignment - 1)) return 0;`n    if (alignment < __alignof__(MEMiExpHeapMBlockHead)) alignment = __alignof__(MEMiExpHeapMBlockHead);"
 }
 [IO.File]::WriteAllText((Join-Path $out ('src\'+$unit+'.c')),$s)
}
