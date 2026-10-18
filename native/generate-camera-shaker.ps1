$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Camera\CameraShaker.cpp" -Raw
$b=$s.IndexOf('void CameraShaker::createSinglyHorizontalTask()')
$e=$s.IndexOf('void CameraShaker::createInfinityTask()',$b)
if($b -lt 0 -or $e -lt 0){throw 'Horizontal shake bounds anchor'}
$section=$s.Substring($b,$e-$b)
$section=$section.Replace('i < NR_VERTICAL_TASKS','i < NR_HORIZONTAL_TASKS')
$s=$s.Substring(0,$b)+$section+$s.Substring($e)
Set-Content "$n\src\generated\CameraShaker.cpp" $s -Encoding UTF8
