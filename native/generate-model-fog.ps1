$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Extract([string]$path,[string]$signature){
 $s=Get-Content -LiteralPath $path -Raw;$start=$s.IndexOf($signature)
 if($start -lt 0){throw "Fog helper anchor missing: $signature"}
 $brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $s.Substring($start,$end-$start)
}
$mix=Extract "$p\Petari\src\Game\Util\DirectDraw.cpp" '    void mixFogColor('
$header=@"
#include <Game/Util/ModelUtil.hpp>
#include <Game/Util/CameraUtil.hpp>
#include <Game/Util/DirectDraw.hpp>
#include <JSystem/J3DGraphBase/J3DSys.hpp>
#include <revolution/mtx.h>
"@
Set-Content -LiteralPath "$n\src\generated\ModelFogKernel.cpp" -Value (($header,'namespace TDDraw {',$mix,'}') -join [Environment]::NewLine)
