$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AudioLib\AudBgmSetting.cpp" -Raw
if(!$s.Contains('{-1, -1, -1, -1, 0, 0, 0, 0}')){throw 'Missing original chord sentinel row'}
$s=$s.Replace('{-1, -1, -1, -1, 0, 0, 0, 0}','{u16(-1), u16(-1), u16(-1), u16(-1), 0, 0, 0, 0}')
Set-Content "$n\src\generated\AudBgmSetting.cpp" $s
