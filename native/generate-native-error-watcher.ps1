$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\GameSystemErrorWatcher.cpp" -Raw
if(!$s.Contains('namespace {')){throw 'Error watcher local namespace anchor missing'}
$s=$s.Replace('namespace {',"#undef ATTRIBUTE_WEAK`n#define ATTRIBUTE_WEAK`nnamespace {")
Set-Content "$n\src\generated\GameSystemErrorWatcher.cpp" $s
