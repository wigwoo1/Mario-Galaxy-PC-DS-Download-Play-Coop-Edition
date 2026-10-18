$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\RaceManager.cpp" -Raw
$a=$s.IndexOf('namespace {');if($a -lt 0){throw 'Race local nerve scope missing'}
$s=$s.Insert($a,"#undef ATTRIBUTE_WEAK"+[Environment]::NewLine+"#define ATTRIBUTE_WEAK"+[Environment]::NewLine)
Set-Content "$n\src\generated\RaceManager.cpp" ('#include <functional>'+[Environment]::NewLine+$s.Replace('std::mem_func(','std::mem_fn('))

