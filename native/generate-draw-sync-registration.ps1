$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\DrawSyncManager.cpp" -Raw
$start=$s.IndexOf('u16 DrawSyncManager::setCallback(')
$end=$s.IndexOf('void DrawSyncManager::reset(', $start)
if($start -lt 0 -or $end -le $start){throw 'Original callback registration anchor missing'}
Set-Content "$n\src\generated\DrawSyncRegistration.cpp" ('#include <Game/System/DrawSyncManager.hpp>'+[Environment]::NewLine+$s.Substring($start,$end-$start))
