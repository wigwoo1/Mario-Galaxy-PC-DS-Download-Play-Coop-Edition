$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\GameAudio\AudStageBgmTable.cpp" -Raw
$start=$s.IndexOf('StageBgm sStageBgmSet[]');$end=$s.IndexOf('};',$start)
if($start -lt 0 -or $end -lt $start){throw 'Missing stage music table'}
$s=$s.Substring(0,$start)+$s.Substring($start,$end-$start).Replace('-1','u32(-1)')+$s.Substring($end)
Set-Content "$n\src\generated\AudStageBgmTable.cpp" $s
