$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$path=Join-Path $n 'src\generated\ResourceHolder.cpp'
$source=[IO.File]::ReadAllText($path)
$original='return mBasResTable->add(pName, pData, true);'
$native='return mBasResTable->add(pName, loadNativeArchiveBas(*mArchive, pData, mHeap), true);'
if(!$source.Contains($native)){
 if([regex]::Matches($source,[regex]::Escape($original)).Count-ne1){throw 'Original BAS resource attachment anchor missing or duplicated'}
 $source='#include "../native_owned_bas.hpp"'+"`n"+$source.Replace($original,$native)
 [IO.File]::WriteAllText($path,$source,[Text.UTF8Encoding]::new($false))
}
