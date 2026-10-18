$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\ModelUtil.cpp" -Raw
$a=$s.IndexOf('    XanimePlayer* newXanimePlayer(');$b=$s.IndexOf('    void updateModelManager(', $a)
if($a -lt 0 -or $b -lt 0){throw 'Original model animation creation helpers missing'}
$prefix=@"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
namespace MR {
"@
($prefix+"`r`n"+$s.Substring($a,$b-$a)+"`r`n}")|Set-Content "$n\src\generated\ModelAnimationCreateKernel.cpp"
