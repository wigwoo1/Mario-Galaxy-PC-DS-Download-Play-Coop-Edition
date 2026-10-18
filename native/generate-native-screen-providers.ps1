$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
foreach($unit in @('IsbnManager','LensFlare','GalaxyMapController','GalaxyMap','GalaxyMapBackground','GalaxyMapSelectButton','PowerStarList','GalaxyMapGalaxyPlain','GalaxyMapGalaxyDetail','GalaxyMapTitle','GalaxyMapIcon','GalaxyMapCometIcon','GalaxyMapDomeIcon','GalaxyMapMarioIcon','GalaxyMapTicoIcon','InformationObserver','MeterLayout','SuddenDeathMeter','SubMeterLayout','PlayerLeft','PlayerActionGuidance')){
$s=Get-Content "$p\Petari\src\Game\Screen\$unit.cpp" -Raw
if($unit -eq 'IsbnManager'){$s=$s.Replace('#include <wstring.h>','#include <cwchar>')}
else {
$space='Native'+$unit
$s="namespace $space {} using namespace $space;`r`n"+$s.Replace('namespace {','namespace '+$space+' {')
if($unit -eq 'LensFlare'){$s=$s.Replace('position += _74;','position.x += _74.x; position.y += _74.y;')}
}
$s=$s.Replace('MR::Functor(this, GalaxyMapController::capture)','MR::Functor(this, &GalaxyMapController::capture)')
if($unit -eq 'GalaxyMap'){$s=$s.Replace('std::mem_func(', 'std::mem_fn(');$s="#include <functional>`r`n"+$s}
if($unit -eq 'GalaxyMapSelectButton'){$s=$s.Replace('mPaneCtrl->mIsSelected != nullptr','mPaneCtrl->mIsSelected')}
if($unit -eq 'GalaxyMap'){
$s=[regex]::Replace($s,'std::binder2nd<\s*std::mem_fun1_t<\s*void,\s*\w+,\s*const nw4r::lyt::DrawInfo&\s*>,\s*const nw4r::lyt::DrawInfo&\s*>\(\s*std::mem_fn\(&\w+::calcAnimForCapture\),\s*rDrawInfo\)','[&rDrawInfo](auto* icon) { icon->calcAnimForCapture(rDrawInfo); }')
$s=$s.Replace('std::bind2nd(std::ptr_fun(MR::drawLayoutWithDrawInfoWithoutProjectionSetup), rDrawInfo)','[&rDrawInfo](auto* icon) { MR::drawLayoutWithDrawInfoWithoutProjectionSetup(icon, rDrawInfo); }')
}
if($unit -eq 'SubMeterLayout'){$s=$s.Replace('MR::startAnim(this, "Appear", nullptr);','MR::startAnim(this, "Appear", 0);')}
Set-Content "$n\src\generated\$unit.cpp" $s
}












