$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
New-Item -ItemType Directory -Force "$n\include\Game\Gravity" | Out-Null
$header=Get-Content "$p\Petari\include\Game\Gravity\PlanetGravityManager.hpp" -Raw
$header='#include <cstdint>'+[Environment]::NewLine+$header.Replace('u32 requester','std::uintptr_t requester')
Set-Content "$n\include\Game\Gravity\PlanetGravityManager.hpp" $header
$s=Get-Content "$p\Petari\src\Game\Gravity\PlanetGravityManager.cpp" -Raw
$s=$s.Replace('u32 gravityType, u32 host','u32 gravityType, std::uintptr_t host').Replace('(u32)(*pGravities)->mHost','reinterpret_cast<std::uintptr_t>((*pGravities)->mHost)')
Set-Content "$n\src\generated\PlanetGravityManagerKernel.cpp" $s
New-Item -ItemType Directory -Force "$n\include\Game\Util" | Out-Null
$header=Get-Content "$p\Petari\include\Game\Util\GravityUtil.hpp" -Raw
Set-Content "$n\include\Game\Util\GravityUtil.hpp" ('#include <cstdint>'+[Environment]::NewLine+$header.Replace('u32 host','std::uintptr_t host'))
$s=Get-Content "$p\Petari\src\Game\Util\GravityUtil.cpp" -Raw
$a=$s.IndexOf('    bool calcGravityVectorOrZero(const NameObj*');$b=$s.IndexOf('};  // namespace',$a)
$helper=$s.Substring($a,$b-$a).Replace('u32 host) NO_INLINE','std::uintptr_t host)').Replace('(u32)pActor','reinterpret_cast<std::uintptr_t>(pActor)')
$body=@('#include <cstring>','#include <Game/Util/GravityUtil.hpp>','#include <Game/Gravity/GravityInfo.hpp>','#include <Game/Gravity/PlanetGravity.hpp>','#include <Game/Gravity/PlanetGravityManager.hpp>','#include <Game/LiveActor/LiveActor.hpp>','#include <Game/Scene/SceneObjHolder.hpp>','namespace {','PlanetGravityManager* getGravityManager(){return MR::getSceneObj<PlanetGravityManager>(SceneObj_PlanetGravityManager);}',$helper,'}','namespace MR {')
foreach($sig in @('    void registerGravity(','    bool calcGravityVector(const LiveActor*','    bool calcGravityVector(const NameObj*','    bool calcGravityVectorOrZero(const LiveActor*','    bool calcGravityVectorOrZero(const NameObj*','    bool calcDropShadowVector(const LiveActor*','    bool calcDropShadowVector(const NameObj*','    bool calcGravityAndDropShadowVector(','    bool calcGravityAndMagnetVector(','    bool calcDropShadowVectorOrZero(','    bool calcGravityAndDropShadowVectorOrZero(','    bool calcAttractMarioLauncherOrZero(','    bool isZeroGravity(')){
 $a=$s.IndexOf($sig,$s.IndexOf('namespace MR {'));if($a -lt 0){throw "Missing gravity anchor $sig"};$b=$s.IndexOf('{',$a);$d=1;$e=$b+1;while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++};$body+=$s.Substring($a,$e-$a).Replace('u32 host','std::uintptr_t host')
}
$constantsStart=$s.IndexOf('    char cRange[]')
$constantsEnd=$s.IndexOf('    PlanetGravityManager* getGravityManager()', $constantsStart)
$helpersStart=$s.IndexOf('    void getJMapInfoArgPlus(')
$helpersEnd=$s.IndexOf('    bool calcGravityVectorOrZero(', $helpersStart)
if($constantsStart -lt 0 -or $helpersStart -lt 0){throw 'Gravity parameter source anchors missing'}
$body+='}'
$body+='namespace {'
$body+=$s.Substring($constantsStart,$constantsEnd-$constantsStart)
$body+=$s.Substring($helpersStart,$helpersEnd-$helpersStart)
$body+='}'
$body+='namespace MR {'
foreach($sig in @('    bool isLightGravity(', '    void settingGravityParamFromJMap(', '    void getJMapInfoGravityType(', '    void getJMapInfoGravityPower(')){
 $a=$s.IndexOf($sig);if($a -lt 0){throw "Missing gravity parameter $sig"};$b=$s.IndexOf('{',$a);$d=1;$e=$b+1;while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++};$body+=$s.Substring($a,$e-$a)
}
$body+='}'
Set-Content "$n\src\generated\GravityRoutingKernel.cpp" ($body -join [Environment]::NewLine)


$umbrella=Get-Content "$p\Petari\include\Game\Util.hpp" -Raw
Set-Content "$n\include\Game\Util.hpp" ($umbrella.Replace('#include "Game/Util/GravityUtil.hpp"','#include <Game/Util/GravityUtil.hpp>'))
$dynamic=Get-Content "$p\Petari\src\Game\LiveActor\DynamicJointCtrl.cpp" -Raw
Set-Content "$n\src\generated\DynamicJointCtrlKernel.cpp" ($dynamic.Replace('#include "Game/Util.hpp"','#include <Game/Util.hpp>'))


