$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\MapUtil.cpp" -Raw
function ExtractBody([string]$sig){
$a=$s.IndexOf($sig);if($a -lt 0){throw "Missing $sig"};$b=$s.IndexOf('{',$a);$depth=1;$i=$b+1
while($depth -gt 0 -and $i -lt $s.Length){if($s[$i] -eq '{'){$depth++};if($s[$i] -eq '}'){$depth--};$i++}
if($depth){throw 'Unbalanced body'};return $s.Substring($a,$i-$a)
}
$out=$s.Substring(0,$s.IndexOf('static HitInfo'))+"static HitInfo mSortBuffer[32];`r`nstatic u32 mSortCount;`r`nnamespace {`r`n"
$out+=(ExtractBody '    const HitInfo* getStrikeInfoCategory(s32 category, u32 index)')+"`r`n"
$out+=(ExtractBody '    bool getFirstPolyOnLineCategory(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,')+"`r`n}`r`nnamespace MR {`r`n"
foreach($name in @('getFirstPolyOnLineToMap','getFirstPolyOnLineToWaterSurface')){
$sig='    bool '+$name+'(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,'
$out+=(ExtractBody $sig)+"`r`n"
}
foreach($name in @('getFirstPolyOnLineToMap','getFirstPolyOnLineToWaterSurface')){
$sig='    bool '+$name+'(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection)'
$out+=(ExtractBody $sig)+"`r`n"
}
foreach($sig in @('    const Triangle* getSortedPoly(u32 sortIndex)', '    const Triangle* getCameraPolyFast(const TVec3f& rStart, const TVec3f& rDirection, const HitSensor* pSensor)', '    const TVec3f* getNormal(const Triangle* pTriangle)', '    bool isWaterPolygon(const Triangle* pTriangle)', '    u32 getNearPolyOnLineSort(const TVec3f& rOrigin, const TVec3f& rStart, const TVec3f& rDirection, const HitSensor* pSensor)', '    bool getSortedPoly(TVec3f* pDst, Triangle* pTriangle, u32 sortIndex)', '    bool isExistMapCollision(const TVec3f& rParam1, const TVec3f& rParam2)', '    bool getFirstPolyOnLineBFast(const TVec3f& rStart, const TVec3f& rDirection, TVec3f* pPos, Triangle* pTriangle)')){
$out+=(ExtractBody $sig)+"`r`n"
}
foreach($sig in @('    bool isWallCodeRebound(const Triangle* pTriangle)', '    bool isBindedGroundSand(const LiveActor* pActor)', '    bool isGroundCodeSand(const Triangle* pTriangle)', '    const char* getFloorCodeString(const Triangle* pTriangle)', '    const char* getWallCodeString(const Triangle* pTriangle)', '    const char* getSoundCodeString(const Triangle* pTriangle)', '    s32 getSoundCodeIndex(const JMapInfoIter& rIter)', '    s32 getWallCodeIndex(const Triangle* pTriangle)', '    s32 getSoundCodeIndex(const Triangle* pTriangle)', '    bool isWallCodeNoAction(const Triangle* pTriangle)', '    bool isCodeSand(const Triangle* pTriangle)', '    bool isBindedGroundDamageFire(const LiveActor* pActor)', '    bool checkStrikePointToMap(const TVec3f& rParam1, HitInfo* pParam2)', '    bool calcMapGround(const TVec3f& rPos, TVec3f* pGround, f32 length)', '    bool isBindedDamageFire(const LiveActor* pActor)', '    u32 getCameraID(const Triangle* pTriangle)', '    bool isBindedGroundWater(const LiveActor* pActor)', '    bool isBindedGroundAreaMove(const LiveActor* pActor)', '    bool isBindedGroundRailMove(const LiveActor* pActor)', '    bool trySetMoveLimitCollision(LiveActor* pActor)', '    bool isFallNextMove(const LiveActor* pActor, f32 param2, f32 param3, f32 param4, const TriangleFilterBase* pParam5)', '    bool isFallNextMove(const TVec3f& rPos, const TVec3f& rVelocity, const TVec3f& rGravity, f32 forwardDistance, f32 upperDistance,', '    void calcVelocityMovingPoint(const Triangle* pTriangle, const TVec3f& rPoint, TVec3f* pVelocity)')){
$out+=(ExtractBody $sig)+"`r`n"
}
$out+="}`r`nnamespace Collision {`r`n"
$out+=(ExtractBody '    s32 checkStrikeLineToSunshade(const TVec3f& rStart, const TVec3f& rDirection, s32 count, const CollisionPartsFilterBase* pPartsFilter,')+"`r`n"
$out+="}`r`n";Set-Content "$n\src\generated\FilteredMapLineQueries.cpp" $out

















