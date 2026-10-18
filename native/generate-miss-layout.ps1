$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Screen\MissLayout.cpp" -Raw
$anchor='namespace {'
if(($s.Split([string[]]@($anchor),[System.StringSplitOptions]::None).Count-1) -ne 1){throw 'Original miss layout namespace anchor changed'}
$s=$s.Replace($anchor,"#undef ATTRIBUTE_WEAK`r`n#define ATTRIBUTE_WEAK`r`n"+$anchor)
Set-Content "$n\src\generated\MissLayout.cpp" $s
