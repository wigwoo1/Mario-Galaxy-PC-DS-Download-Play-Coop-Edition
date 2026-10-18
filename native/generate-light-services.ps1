$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\LightFunction.cpp" -Raw
function Extract([string]$signature){
 $start=$s.IndexOf($signature);if($start -lt 0){throw "Original light service anchor missing: $signature"};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$s.Substring($start,$end-$start)
}
$parts=@('#include <revolution/gx/GXLighting.h>','#include <Game/Map/LightFunction.hpp>','#include <Game/Map/LightDirector.hpp>','#include <Game/AreaObj/LightAreaHolder.hpp>','#include <Game/Scene/SceneObjHolder.hpp>','#include <Game/Util/ObjUtil.hpp>','#include <Game/Util/JMapInfo.hpp>','#include <cstdio>')
foreach($signature in @('void LightFunction::initLightData()','ResourceHolder* LightFunction::loadLightArchive()','s32 LightFunction::createLightDataParser(','s32 LightFunction::createZoneDataParser(','const char* LightFunction::getDefaultAreaLightName()','s32 LightFunction::getDefaultStepInterpolate()','void LightFunction::registerPlayerLightCtrl(','void LightFunction::registerLightAreaHolder(','bool LightFunction::tryFindNewAreaLightID(','AreaLightInfo* LightFunction::getAreaLightInfo(')){$parts+=(Extract $signature)}
Set-Content "$n\src\generated\LightServicesKernel.cpp" ($parts -join [Environment]::NewLine)

