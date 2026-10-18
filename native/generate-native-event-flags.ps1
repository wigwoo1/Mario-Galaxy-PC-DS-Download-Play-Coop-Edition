$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\GameEventFlagChecker.cpp" -Raw
$start=$s.IndexOf('bool GameEventFlagChecker::canOn(')
$end=$s.IndexOf('bool GameEventFlagChecker::isOn(', $start)
$body=$s.Substring($start,$end-$start)
foreach($case in @('GalaxyOpenStar','EventFlag','StarPiece','EventValueIsZero')){
 $pattern='(case GameEventFlag::Type_'+$case+':)((?s:.*?))(?=    case GameEventFlag::|    default:)'
 $body=[regex]::Replace($body,$pattern,'$1 {'+[Environment]::NewLine+'$2    }'+[Environment]::NewLine)
}
$s=$s.Substring(0,$start)+$body+$s.Substring($end)
Set-Content "$n\src\generated\GameEventFlagChecker.cpp" $s

