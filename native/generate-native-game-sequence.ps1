$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\GameSequenceProgress.cpp" -Raw
$a=$s.IndexOf('namespace {');if($a -lt 0){throw 'Missing local nerve namespace'}
$s=$s.Insert($a,"#undef ATTRIBUTE_WEAK`r`n#define ATTRIBUTE_WEAK`r`n")
Set-Content "$n\src\generated\GameSequenceProgress.cpp" $s
