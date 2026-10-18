$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\src\Game\Util\ModelUtil.cpp" -Raw
$signature='    bool isBckPlaying(XanimePlayer*'
$start=$source.IndexOf($signature)
if($start -lt 0){throw 'Missing original animation-playing function'}
$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1
while($depth){if($source[$end] -eq '{'){$depth++}elseif($source[$end] -eq '}'){$depth--};$end++}
$body=@('#include <Game/Util/ModelUtil.hpp>','#include <Game/Animation/XanimePlayer.hpp>','namespace MR {',$source.Substring($start,$end-$start),'}')
Set-Content "$n\src\generated\AnimationPlayingKernel.cpp" ($body -join [Environment]::NewLine)
