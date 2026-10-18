$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\LightFunction.cpp" -Raw
function Extract([string]$signature){
 $start=$s.IndexOf($signature);if($start -lt 0){throw "Original light loader anchor missing: $signature"}
 $brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $s.Substring($start,$end-$start).Replace(' NO_INLINE','').Replace('GXLightObj lightObj;','GXLightObj lightObj{};').Replace('GXLightObj obj;','GXLightObj obj{};')
}
$parts=@('#include <revolution/gx/GXLighting.h>','#include <Game/Map/LightFunction.hpp>','#include <Game/Map/LightPointCtrl.hpp>','#include <Game/Util/CameraUtil.hpp>','#include <Game/Util/Color.hpp>','#include <Game/Util/MathUtil.hpp>',(Extract 'void LightFunction::initLightRegisterAll()'),'namespace {')
foreach($signature in @('    void blendActorLightPos(','    void loadLightDiffuse(','    inline void loadCameraLight(','    void loadLightInfoDiffuse(')){$parts+=(Extract $signature)}
$parts+='}'
foreach($signature in @('void LightFunction::loadActorLightInfo(','void LightFunction::loadPointLightInfo(','void LightFunction::loadLightInfoCoin(','void LightFunction::loadAllLightWhite(','void LightFunction::blendActorLightInfo(','void LightFunction::calcLightWorldPos(')){$parts+=(Extract $signature)}
Set-Content "$n\src\generated\ActorLightLoadKernel.cpp" ($parts -join [Environment]::NewLine)




