$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Extract([string]$source,[string]$signature){
 $start=$source.IndexOf($signature);if($start -lt 0){throw "Original area-light anchor missing: $signature"}
 $brace=$source.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($source[$end] -eq '{'){$depth++}elseif($source[$end] -eq '}'){$depth--};$end++}
 $source.Substring($start,$end-$start).Replace(' NO_INLINE','')
}
$source=Get-Content "$p\Petari\src\Game\Map\LightFunction.cpp" -Raw
$parts=@('#include <revolution/gx/GXLighting.h>','#include <Game/Map/LightFunction.hpp>','#include <Game/Util/ObjUtil.hpp>','#include <Game/Util/JMapInfo.hpp>','#include <cstdio>','namespace {')
foreach($signature in @('    void getDataLightInfo(','    void getDataActorLightInfo(')){$parts+=(Extract $source $signature)}
$parts+='}';$parts+=(Extract $source 'void LightFunction::getAreaLightLightData(')
Set-Content "$n\src\generated\AreaLightDataKernel.cpp" ($parts -join [Environment]::NewLine)
# Narrow original CSV helpers are test dependencies only; full ObjUtil already provides them.
$source=Get-Content "$p\Petari\src\Game\Util\ObjUtil.cpp" -Raw
$parts=@('#include <revolution/gx/GXLighting.h>','#include <Game/Util/ObjUtil.hpp>','#include <Game/Util/JMapInfo.hpp>','#include <cstdio>')
foreach($signature in @('void MR::getCsvDataStr(','void MR::getCsvDataS32(','void MR::getCsvDataU8(','void MR::getCsvDataF32(','void MR::getCsvDataBool(','void MR::getCsvDataVec(','void MR::getCsvDataColor(')){$parts+=(Extract $source $signature)}
Set-Content "$n\src\generated\CsvLightFixtureKernel.cpp" ($parts -join [Environment]::NewLine)
$holder=Get-Content "$p\Petari\src\Game\Map\LightDataHolder.cpp" -Raw
$holder=$holder.Replace('#include "Game/Util.hpp"','#include "Game/Util/StringUtil.hpp"')
Set-Content "$n\src\generated\LightDataHolderKernel.cpp" ('#include <revolution/gx/GXLighting.h>'+[Environment]::NewLine+$holder)
