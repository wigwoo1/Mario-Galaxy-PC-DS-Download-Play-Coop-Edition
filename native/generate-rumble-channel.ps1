$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\WPadRumble.cpp" -Raw
$start=$s.IndexOf('void RumbleChannel::clear()');$end=$s.IndexOf('WPadRumble::WPadRumble(')
if($start -lt 0 -or $end -le $start){throw 'Original rumble channel extraction boundary missing'}
$out='#include "Game/System/WPadRumble.hpp"'+"`r`n"+'#include "Game/System/WPadRumbleData.hpp"'+"`r`n"+$s.Substring($start,$end-$start)
Set-Content "$n\src\generated\RumbleChannelKernel.cpp" $out
