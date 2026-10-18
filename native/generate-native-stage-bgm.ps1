$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\GameAudio\AudStageBgmWrap.cpp" -Raw
# Preserve unsigned invalid sound IDs explicitly under C++17 narrowing rules.
$s=$s.Replace('{nullptr, nullptr, -1, -1}', '{nullptr, nullptr, u32(-1), -1}')
$start=$s.IndexOf('const u32 sMASenarioBgmData');$end=$s.IndexOf('};',$start)
if($start -lt 0 -or $end -lt $start){throw 'Missing original scenario music table'}
$part=$s.Substring($start,$end-$start).Replace('-1','u32(-1)')
$s=$s.Substring(0,$start)+$part+$s.Substring($end)
Set-Content "$n\src\generated\AudStageBgmWrap.cpp" $s
