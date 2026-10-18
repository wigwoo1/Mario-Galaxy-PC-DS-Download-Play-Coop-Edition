$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Player\PlayerEvent.cpp" -Raw -Encoding utf8
$start=$s.IndexOf('void EventSequencer::init(');$end=$s.IndexOf('void EventSequencer::movement()', $start)
if($start -lt 0 -or $end -lt $start){throw 'Original init not found'}
$body=$s.Substring($start,$end-$start).Replace('    mHashTable->sort();','    MR::connectToSceneMapObjMovement(this);')
$includes=$s.Substring(0,$s.IndexOf('EventSequencer::EventSequencer()'))
Set-Content "$n\src\generated\ProductionEventSequencerInit.cpp" ($includes+$body) -Encoding utf8
