$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\NPC\TalkState.cpp" -Raw
foreach($pair in @(@('_14 = nullptr;','_14 = 0;'),@('_10 = nullptr;','_10 = 0;'),@('_1C != nullptr','_1C != 0'))){if(!$s.Contains($pair[0])){throw 'Original TalkState zero spelling missing'};$s=$s.Replace($pair[0],$pair[1])}
$s|Set-Content "$n\src\generated\TalkState.cpp"
