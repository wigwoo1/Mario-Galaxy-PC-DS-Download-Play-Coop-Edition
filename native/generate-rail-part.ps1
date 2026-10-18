$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\RailPart.cpp" -Raw
$s=$s.Replace('const register TVec3f&','const TVec3f&').Replace('C_VECMag(&mCtrlDegree1)','C_VECMag(static_cast<const Vec*>(mCtrlDegree1))')
$s=$s.Replace('rPoint1.epsilonEquals(rPoint1Ctrl, 0.1f)','nativeRailEpsilonEquals(rPoint1, rPoint1Ctrl, 0.1f)').Replace('rPoint2.epsilonEquals(rPoint2Ctrl, 0.1f)','nativeRailEpsilonEquals(rPoint2, rPoint2Ctrl, 0.1f)')
$helper=@"
#include <JSystem/JGeometry/TVec.hpp>
namespace { bool nativeRailEpsilonEquals(const TVec3f& a,const TVec3f& b,float e) {
 const float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;
 return -e<=x && x<=e && -e<=y && y<=e && -e<=z && z<=e;
}}
"@
Set-Content "$n\src\generated\RailPart.cpp" ($helper+"`n"+$s)
