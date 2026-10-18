$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\LightZoneDataHolder.cpp" -Raw
function Extract([string]$signature){
 $start=$s.IndexOf($signature);if($start -lt 0){throw "Original zone-light anchor missing: $signature"};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$s.Substring($start,$end-$start)
}
$parts=@('#include <revolution/gx/GXLighting.h>','#include <Game/Map/LightZoneDataHolder.hpp>','#include <Game/AreaObj/LightArea.hpp>','#include <Game/Map/LightFunction.hpp>','#include <Game/Util/SceneUtil.hpp>','#include <Game/Util/ObjUtil.hpp>','#include <Game/Util/JMapInfo.hpp>')
foreach($signature in @('ZoneLightID::ZoneLightID()','void ZoneLightID::clear()','bool ZoneLightID::isOutOfArea()','bool ZoneLightID::isTargetArea(','LightZoneInfo::LightZoneInfo()','const char* LightZoneInfo::getAreaLightNameInZoneData(','LightZoneDataHolder::LightZoneDataHolder()','void LightZoneDataHolder::initZoneData()','const char* LightZoneDataHolder::getAreaLightNameInZoneData(','const char* LightZoneDataHolder::getDefaultStageAreaLightName()','void LightZoneInfo::init(')){$parts+=(Extract $signature)}
Set-Content "$n\src\generated\ZoneLightDataKernel.cpp" ($parts -join [Environment]::NewLine)

