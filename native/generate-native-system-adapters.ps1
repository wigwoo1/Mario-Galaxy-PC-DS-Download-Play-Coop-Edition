$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function ReplaceOnce([string]$Text,[string]$Old,[string]$New,[string]$Name){$count=([regex]::Matches($Text,[regex]::Escape($Old))).Count;if($count -ne 1){throw "$Name anchor count $count (expected 1)"};return $Text.Replace($Old,$New)}
function RemoveCxxDefinitionsAtLines([string]$Text,[int[]]$LineNumbers,[string]$Name){
    $lines=[System.Collections.Generic.List[string]]::new()
    foreach($line in [regex]::Split($Text,"`r`n|`n")){$lines.Add($line)}
    foreach($lineNumber in ($LineNumbers|Sort-Object -Descending -Unique)){
        $start=$lineNumber-1
        if($start -lt 0 -or $start -ge $lines.Count -or -not $lines[$start].Contains('{')){throw "$Name definition anchor mismatch at line ${lineNumber}: $($lines[$start])"}
        $depth=0;$seen=$false;$end=-1
        for($i=$start;$i -lt $lines.Count;$i++){
            foreach($ch in $lines[$i].ToCharArray()){
                if($ch -eq '{'){$depth++;$seen=$true}
                elseif($ch -eq '}'){$depth--;if($seen -and $depth -eq 0){$end=$i;break}}
            }
            if($end -ge 0){break}
        }
        if($end -lt $start){throw "$Name function body did not close from line $lineNumber"}
        $lines.RemoveRange($start,$end-$start+1);$lines.Insert($start,'')
    }
    return [string]::Join("`r`n",$lines)
}
$star=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Util\StarPointerUtil.cpp")
$star=ReplaceOnce $star 'const TVec2f& posB = posA.subInline(controller->mScreenVel);' 'TVec2f posB(posA.x - controller->mScreenVel.x, posA.y - controller->mScreenVel.y);' 'star pointer velocity subtraction'
$star=ReplaceOnce $star 'screenVel.scale(1.0f / rad2D);' "screenVel.x *= 1.0f / rad2D;`r`n        screenVel.y *= 1.0f / rad2D;" 'star pointer velocity normalization'
$star=ReplaceOnce $star 'screenVel.scale(0.726f / speed);' "screenVel.x *= 0.726f / speed;`r`n            screenVel.y *= 0.726f / speed;" 'star pointer velocity clamp'
$star=ReplaceOnce $star 'screenPos -= targetPos;' "screenPos.x -= targetPos.x;`r`n        screenPos.y -= targetPos.y;" 'star pointer position subtraction'
[System.IO.File]::WriteAllText("$n\src\generated\StarPointerUtil.cpp",$star,[System.Text.UTF8Encoding]::new($false))
$heap=[System.IO.File]::ReadAllText("$p\Petari\src\Game\System\GameSystemStationedArchiveLoader.cpp")
$heap=ReplaceOnce $heap 'napa->alloc(0x10000, nullptr);' 'JKRHeap::alloc(0x10000, -4, napa);' 'Napa heap allocation'
$heap=ReplaceOnce $heap 'gddr->alloc(0x10000, nullptr);' 'JKRHeap::alloc(0x10000, -4, gddr);' 'GDDR heap allocation'
[System.IO.File]::WriteAllText("$n\src\generated\GameSystemStationedArchiveLoader.cpp",$heap,[System.Text.UTF8Encoding]::new($false))
$marioDraw=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Player\MarioActorSpecialDraw.cpp")
$randomCall='MR::getRandom(0L, 8L)'
if(([regex]::Matches($marioDraw,[regex]::Escape($randomCall))).Count -ne 2){throw 'MarioActorSpecialDraw random overload anchor count changed'}
$marioDraw=$marioDraw.Replace($randomCall,'MR::getRandom(static_cast<s32>(0), static_cast<s32>(8))')
$marioDraw=& (Join-Path $PSScriptRoot 'adapt-player-private-writers.ps1') -SourceText $marioDraw -Unit MarioActorSpecialDraw
[System.IO.File]::WriteAllText("$n\src\generated\MarioActorSpecialDraw.cpp",$marioDraw,[System.Text.UTF8Encoding]::new($false))
$demo=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Util\DemoUtil.cpp")
$demo=ReplaceOnce $demo "LiveActor* getTalkingActor() {`r`n        if (isExistSceneObj(SceneObj_TalkDirector) == false) {`r`n            return false;" "LiveActor* getTalkingActor() {`r`n        if (isExistSceneObj(SceneObj_TalkDirector) == false) {`r`n            return nullptr;" 'DemoUtil pointer null return'
[System.IO.File]::WriteAllText("$n\src\generated\DemoUtilRuntime.cpp",$demo,[System.Text.UTF8Encoding]::new($false))
$demoDirector=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Demo\DemoDirector.cpp")
$demoDirector=ReplaceOnce $demoDirector 'info.mFrameType == nullptr' 'info.mFrameType == DemoStartInfo::CinemaFrameType_0' 'DemoDirector frame enum'
[System.IO.File]::WriteAllText("$n\src\generated\DemoDirectorRuntime.cpp",$demoDirector,[System.Text.UTF8Encoding]::new($false))
$demoExecutor=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Demo\DemoExecutor.cpp")
foreach($method in @('DemoSheetKeeperBase::update','DemoSheetKeeperBase::start','DemoSheetKeeperBase::end','DemoTalkAnimCtrl::updateDemo','DemoTalkAnimCtrl::startDemo')){
    $className=$method.Split('::')[0];$methodName=$method.Split('::')[1]
    $demoExecutor=ReplaceOnce $demoExecutor ('std::mem_func(&'+$method+')') ('[]('+ $className+'* pCtrl) { pCtrl->'+$methodName+'(); }') ('DemoExecutor member callback '+$method)
}
$bindPattern='std::binder2nd<\s*std::mem_fun1_t<\s*void,\s*DemoTalkAnimCtrl,\s*const char\*\s*>,\s*const char\*\s*>\(\s*std::mem_func\(&DemoTalkAnimCtrl::setupStartDemoPart\),\s*(pParam3|pParam1)\)'
if([regex]::Matches($demoExecutor,$bindPattern).Count -ne 3){throw 'DemoExecutor setupStartDemoPart binder count changed'}
$demoExecutor=[regex]::Replace($demoExecutor,$bindPattern,'[$1](DemoTalkAnimCtrl* pCtrl) { pCtrl->setupStartDemoPart($1); }')
$demoExecutor=ReplaceOnce $demoExecutor 'std::ptr_fun(MR::validateClipping)' '[](LiveActor* pActor) { MR::validateClipping(pActor); }' 'DemoExecutor validate clipping callback'
[System.IO.File]::WriteAllText("$n\src\generated\DemoExecutorRuntime.cpp",$demoExecutor,[System.Text.UTF8Encoding]::new($false))
$demoHolder=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Demo\DemoStartRequestHolder.cpp")
$holderPattern='(?s)(template\s*<>\s*MR::FixedRingBuffer< const DemoStartInfo\*, 16 >::iterator::iterator\(.*\z)'
$holderMatch=[regex]::Match($demoHolder,$holderPattern)
if(!$holderMatch.Success){throw 'DemoStartRequestHolder specialization block not found'}
$holderSpecializations=$holderMatch.Groups[1].Value
$holderSpecializations=ReplaceOnce $holderSpecializations '    ++mEnd;' "    mEnd.mHead++;`r`n    if (mEnd.mEnd <= mEnd.mHead) {`r`n        mEnd.mHead = mEnd.mTail;`r`n    }" 'DemoStartRequestHolder inline ring advance'
$demoHolder=[regex]::Replace($demoHolder,$holderPattern,'')
$demoHolder=ReplaceOnce $demoHolder 'DemoStartInfo::DemoStartInfo()' ($holderSpecializations+"`r`n`r`nDemoStartInfo::DemoStartInfo()") 'DemoStartRequestHolder specialization order'
[System.IO.File]::WriteAllText("$n\src\generated\DemoStartRequestHolderRuntime.cpp",$demoHolder,[System.Text.UTF8Encoding]::new($false))
$cameraUtil=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Util\CameraUtil.cpp")
$cameraUtil="#include <Native/CameraAnimationPointer.hpp>`r`n"+$cameraUtil
$cameraUtil=ReplaceOnce $cameraUtil 'pScreenPos->scale(1.0f / pos.w, *pos.toTVec3());' 'TVec3f projected(pos.x, pos.y, pos.z); pScreenPos->scale(1.0f / pos.w, projected);' 'CameraUtil TVec4 projection'
$cameraUtil=ReplaceOnce $cameraUtil '(GXProjectionType) nullptr' 'static_cast<GXProjectionType>(0)' 'CameraUtil projection type enum'
$cameraUtil=ReplaceOnce $cameraUtil 'reinterpret_cast< s32 >(pAnimData)' 'Native::storeCameraAnimationData(pAnimData)' 'CameraUtil animation pointer handle'
$duplicateCameraHelpers=@(
    '(?ms)^    f32 calcCameraDistanceZ\(const TVec3f& rPos\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    f32 getFovy\(\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    bool isSubjectiveCameraOnForObjClipping\(\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    bool isExistMirrorCamera\(\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    const TPos3f& getMirrorCameraViewMtx\(\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    const TPos3f& getMirrorModelTexMtx\(\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    const TPos3f& getCameraViewMtx\(\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    const TPos3f& getCameraInvViewMtx\(\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    const TProj3f& getCameraProjectionMtx\(\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    f32 getNearZ\(\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    f32 getFarZ\(\) \{\r?\n.*?^    \}\r?\n'
)
foreach($pattern in $duplicateCameraHelpers){
    $matches=[regex]::Matches($cameraUtil,$pattern)
    if($matches.Count -ne 1){throw "Expected one duplicate CameraUtil helper for pattern $pattern; found $($matches.Count)"}
    $cameraUtil=[regex]::Replace($cameraUtil,$pattern,'',1)
}
[System.IO.File]::WriteAllText("$n\src\generated\CameraUtilRuntime.cpp",$cameraUtil,[System.Text.UTF8Encoding]::new($false))
$cameraManGame=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Camera\CameraManGame.cpp")
$cameraManGame="#include <Native/CameraAnimationPointer.hpp>`r`n"+$cameraManGame
$cameraManGame=ReplaceOnce $cameraManGame 'reinterpret_cast< s32 >(data)' 'Native::storeCameraAnimationData(data)' 'CameraManGame animation pointer handle'
[System.IO.File]::WriteAllText("$n\src\generated\CameraManGameRuntime.cpp",$cameraManGame,[System.Text.UTF8Encoding]::new($false))
$camTranslator=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Camera\CamTranslatorAnim.cpp")
$camTranslator="#include <Native/CameraAnimationPointer.hpp>`r`n"+$camTranslator
$camTranslator=$camTranslator.Replace('reinterpret_cast< u8* >(pChunk->mGeneralParam->mNum1)','Native::resolveCameraAnimationData(pChunk->mGeneralParam->mNum1)')
[System.IO.File]::WriteAllText("$n\src\generated\CamTranslatorAnimRuntime.cpp",$camTranslator,[System.Text.UTF8Encoding]::new($false))
$originalInclude="$n\build\original-include\Game\Player"
New-Item -ItemType Directory -Force $originalInclude|Out-Null
Copy-Item -LiteralPath "$p\Petari\include\Game\Player\PlayerEvent.hpp" -Destination "$originalInclude\PlayerEvent.hpp" -Force
$cameraActor=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Player\MarioActorCamera.cpp")
[System.IO.File]::WriteAllText("$n\src\generated\MarioActorCameraRuntime.cpp",$cameraActor,[System.Text.UTF8Encoding]::new($false))
$marioCollision=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Player\MarioCollision.cpp")
$marioCollision=ReplaceOnce $marioCollision 'startPadVib(0UL);' 'startPadVib(static_cast<u32>(0));' 'MarioCollision vibration overload'
[System.IO.File]::WriteAllText("$n\src\generated\MarioCollisionRuntime.cpp",$marioCollision,[System.Text.UTF8Encoding]::new($false))
$npcUtil=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Util\NPCUtil.cpp")
$npcUtil=ReplaceOnce $npcUtil 'blendVec(&pActor->mPosition, pActor->mPosition, rPos, posBlend);' 'pActor->mPosition = pActor->mPosition * (1.0f - posBlend) + rPos * posBlend;' 'NPCUtil position blend host overload'
$npcUtil=ReplaceOnce $npcUtil 'getRandom(0L, 3L)' 'getRandom(static_cast<s32>(0), static_cast<s32>(3))' 'NPCUtil random integer overload'
[System.IO.File]::WriteAllText("$n\src\generated\NPCUtilRuntime.cpp",$npcUtil,[System.Text.UTF8Encoding]::new($false))
$starDirector=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Screen\StarPointerDirector.cpp")
$starDirector=ReplaceOnce $starDirector 'mTransHolder->mProjMtx.setInline(MR::getCameraProjectionMtx());' 'mTransHolder->mProjMtx = MR::getCameraProjectionMtx();' 'star pointer camera projection host copy'
[System.IO.File]::WriteAllText("$n\src\generated\StarPointerDirectorRuntime.cpp",$starDirector,[System.Text.UTF8Encoding]::new($false))
$mapUtil=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Util\MapUtil.cpp")
$mapUtilDuplicateFunctions=@(
    '(?ms)^    bool isWallPolygon\(const TVec3f& rParam1, const TVec3f& rParam2\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    bool isFloorPolygon\(const TVec3f& rParam1, const TVec3f& rParam2\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    bool isFloorPolygonCos\(const TVec3f& rParam1, const TVec3f& rParam2, f32 param3\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    bool isWallPolygon\(f32 param1\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    bool isFloorPolygon\(f32 param1\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    s32 checkStrikeBallToMap\(const TVec3f& rCenter, f32 radius, const CollisionPartsFilterBase\* pPartsFilter,\r?\n.*?^    \}\r?\n',
    '(?ms)^    s32 checkStrikeBallToMapWithMovingReaction\(const TVec3f& rCenter, f32 radius, const CollisionPartsFilterBase\* pPartsFilter,\r?\n.*?^    \}\r?\n',
    '(?ms)^    const HitInfo\* getStrikeInfoMap\(u32 index\) \{\r?\n.*?^    \}\r?\n',
    '(?ms)^    u32 getStrikeInfoNumMap\(\) \{\r?\n.*?^    \}\r?\n'
)
foreach($pattern in $mapUtilDuplicateFunctions){
    $matches=[regex]::Matches($mapUtil,$pattern)
    if($matches.Count -ne 1){throw "Expected one overlapping MapUtil function; found $($matches.Count) for $pattern"}
    $mapUtil=[regex]::Replace($mapUtil,$pattern,'',1)
}
[System.IO.File]::WriteAllText("$n\src\generated\MapUtilRuntime.cpp",$mapUtil,[System.Text.UTF8Encoding]::new($false))
$npcActor=[System.IO.File]::ReadAllText("$p\Petari\src\Game\NPC\NPCActor.cpp")
$npcActor=ReplaceOnce $npcActor '&calcJointScale' '&NPCActor::calcJointScale' 'NPCActor joint callback member qualification'
$npcActor=ReplaceOnce $npcActor '_A0.getEuler(euler);' 'TPos3f quatMatrix; quatMatrix.setQuat(_A0); quatMatrix.getEuler(euler);' 'NPCActor quaternion Euler conversion'
[System.IO.File]::WriteAllText("$n\src\generated\NPCActorRuntime.cpp",$npcActor,[System.Text.UTF8Encoding]::new($false))
$talkBalloon=[System.IO.File]::ReadAllText("$p\Petari\src\Game\NPC\TalkBalloon.cpp")
$talkBalloon=$talkBalloon.Replace('fmin(', 'balloonFmin(').Replace('fmax(', 'balloonFmax(')
[System.IO.File]::WriteAllText("$n\src\generated\TalkBalloonRuntime.cpp",$talkBalloon,[System.Text.UTF8Encoding]::new($false))
$talkMessageCtrl=[System.IO.File]::ReadAllText("$p\Petari\src\Game\NPC\TalkMessageCtrl.cpp")
$talkMessageCtrl=$talkMessageCtrl.Replace('->operator()(nodeEvent->mUnknown) == nullptr','->operator()(nodeEvent->mUnknown) == false')
[System.IO.File]::WriteAllText("$n\src\generated\TalkMessageCtrlRuntime.cpp",$talkMessageCtrl,[System.Text.UTF8Encoding]::new($false))
$talkState=[System.IO.File]::ReadAllText("$p\Petari\src\Game\NPC\TalkState.cpp")
$talkState=ReplaceOnce $talkState '_14 = nullptr;' '_14 = 0;' 'TalkStateEvent host null integer'
$talkState=ReplaceOnce $talkState '_10 = nullptr;' '_10 = 0;' 'TalkStateEvent host null integer second field'
$talkState=ReplaceOnce $talkState 'if (_1C != nullptr)' 'if (_1C != 0)' 'TalkStateEvent host null boolean'
[System.IO.File]::WriteAllText("$n\src\generated\TalkStateRuntime.cpp",$talkState,[System.Text.UTF8Encoding]::new($false))
$multiSceneEffects=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Scene\MultiSceneEffectKeeper.cpp")
$multiSceneEffects=ReplaceOnce $multiSceneEffects 'std::for_each(mEmitter.begin(), mEmitter.end(), std::mem_func(&MultiEmitter::deleteEmitter));' 'for (MultiEmitter* pEmitter : mEmitter) { pEmitter->deleteEmitter(); }' 'MultiSceneEffectKeeper delete lambda'
$forceDeletePattern='(?s)    std::for_each\(\s*mEmitter\.begin\(\), mEmitter\.end\(\),\s*std::binder2nd<.*?pSystem\)\);'
$forceDeleteMatches=[regex]::Matches($multiSceneEffects,$forceDeletePattern)
$forceDeletePatternAnchor=$forceDeleteMatches.Count
if($forceDeletePatternAnchor -ne 1){throw "MultiScene force-delete pattern count $forceDeletePatternAnchor"}
$multiSceneEffects=[regex]::Replace($multiSceneEffects,$forceDeletePattern,'    for (MultiEmitter* pEmitter : mEmitter) { pEmitter->forceDelete(pSystem); }',1)
$multiSceneEffects=ReplaceOnce $multiSceneEffects 'std::for_each(mEmitter.begin(), mEmitter.end(), std::mem_func(&MultiEmitter::playCalcAndDeleteForeverEmitter));' 'for (MultiEmitter* pEmitter : mEmitter) { pEmitter->playCalcAndDeleteForeverEmitter(); }' 'MultiSceneEffectKeeper clear lambda'
$multiSceneEffects=ReplaceOnce $multiSceneEffects 'std::find_if(mEmitter.begin(), mEmitter.end(),
                     std::binder2nd< std::const_mem_fun1_t< bool, MultiEmitter, u16 >, u16 >(std::mem_func(&MultiEmitter::isEqualName), hash))' 'std::find_if(mEmitter.begin(), mEmitter.end(), [hash](const MultiEmitter* pEmitter) { return pEmitter->isEqualName(hash); })' 'MultiSceneEffectKeeper find predicate lambda'
[System.IO.File]::WriteAllText("$n\src\generated\MultiSceneEffectKeeperRuntime.cpp",$multiSceneEffects,[System.Text.UTF8Encoding]::new($false))
$mathRuntime=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Util\MathUtil.cpp")
$mathRuntime=ReplaceOnce $mathRuntime 'pQuat->setEulerDegree(rRot.x, rRot.y, rRot.z);' 'pQuat->setEuler(rRot.x * PI_180, rRot.y * PI_180, rRot.z * PI_180);' 'MathUtil quaternion degree conversion'
$mathRuntime=ReplaceOnce $mathRuntime 'PSVECNormalize(pVec, pVec);' 'if (pVec->length() > 0.0f) { pVec->scale(1.0f / pVec->length()); } else { pVec->zero(); }' 'MathUtil vector normalization'
$mathRuntime=ReplaceOnce $mathRuntime 'return PSVECKillElement(rSrc, rKillDir, pDst);' "f32 dot = rSrc.dot(rKillDir);`r`n        *pDst = rSrc - rKillDir * dot;`r`n        return dot;" 'MathUtil vector projection removal'
$mathRuntime=[regex]::Replace($mathRuntime,'\bregister\s+','')
$vecScalePattern='(?s)    void vecScaleAdd\(const TVec3f\* pA1, const TVec3f\* pA2, f32 a3\) \{\s*#ifdef __MWERKS__.*?#endif\s*\}'
if([regex]::Matches($mathRuntime,$vecScalePattern).Count -ne 1){throw 'MathUtil vecScaleAdd body anchor mismatch'}
$mathRuntime=[regex]::Replace($mathRuntime,$vecScalePattern,"    void vecScaleAdd(const TVec3f* pA1, const TVec3f* pA2, f32 a3) {`r`n        TVec3f result = *pA1 + *pA2 * a3;`r`n        *const_cast<TVec3f*>(pA1) = result;`r`n    }",1)
$vecBlendPattern='(?s)    void PSvecBlend\(const TVec3f\* pA1, const TVec3f\* pA2, TVec3f\* pA3, f32 a4, f32 a5\) \{\s*#ifdef __MWERKS__.*?#endif\s*\}'
if([regex]::Matches($mathRuntime,$vecBlendPattern).Count -ne 1){throw 'MathUtil PSvecBlend body anchor mismatch'}
$mathRuntime=[regex]::Replace($mathRuntime,$vecBlendPattern,"    void PSvecBlend(const TVec3f* pA1, const TVec3f* pA2, TVec3f* pA3, f32 a4, f32 a5) {`r`n        *pA3 = *pA1 * a4 + *pA2 * a5;`r`n    }",1)
$mathRuntime=ReplaceOnce $mathRuntime 'PSVECNormalize(vA, vA);' 'vA.normalize();' 'MathUtil blend normalize A'
$mathRuntime=ReplaceOnce $mathRuntime 'PSVECScale(vA, vA, 1.0f - rate);' 'vA.scale(1.0f - rate);' 'MathUtil blend scale A'
$mathRuntime=ReplaceOnce $mathRuntime 'PSVECNormalize(vB, vB);' 'vB.normalize();' 'MathUtil blend normalize B'
$mathRuntime=ReplaceOnce $mathRuntime 'PSVECScale(vB, vB, rate);' 'vB.scale(rate);' 'MathUtil blend scale B'
$mathRuntime=ReplaceOnce $mathRuntime 'PSVECAdd(vA, vB, pVec);' '*pVec = vA + vB;' 'MathUtil blend addition'
$mathRuntime=ReplaceOnce $mathRuntime '__asm { frsqrte inverse, value }' 'inverse = 1.0f / std::sqrt(value);' 'MathUtil host square root estimate'
$mathDuplicateLines=@(53,67,82,89,93,97,105,121,129,141,145,153,158,163,220,241,410,422,522,528,558,658,741,748,766,781,807,817,870,885,953,977,989,1181,1198,1202,1206,1228,1290,1297,1303,1351,1371,1399)
$mathRuntime=RemoveCxxDefinitionsAtLines $mathRuntime $mathDuplicateLines 'MathUtil duplicate provider pruning'
[System.IO.File]::WriteAllText("$n\src\generated\MathUtilRuntime.cpp",$mathRuntime,[System.Text.UTF8Encoding]::new($false))
$mtxRuntime=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Util\MtxUtil.cpp")
$mtxRuntime=ReplaceOnce $mtxRuntime 'pDst->setScale(rScale.x, rScale.y, rScale.z);' "pDst->mMtx[0][0] = rScale.x;`r`n        pDst->mMtx[0][1] = 0.0f;`r`n        pDst->mMtx[0][2] = 0.0f;`r`n        pDst->mMtx[1][0] = 0.0f;`r`n        pDst->mMtx[1][1] = rScale.y;`r`n        pDst->mMtx[1][2] = 0.0f;`r`n        pDst->mMtx[2][0] = 0.0f;`r`n        pDst->mMtx[2][1] = 0.0f;`r`n        pDst->mMtx[2][2] = rScale.z;" 'MtxUtil diagonal scaling'
$mtxDuplicateLines=@(17,42,49,53,72,97,101,246,271,331,362,374,408,420,425,438,474,575,596,641,677,683,689,701,707,815,825,835)
$mtxRuntime=RemoveCxxDefinitionsAtLines $mtxRuntime $mtxDuplicateLines 'MtxUtil duplicate provider pruning'
[System.IO.File]::WriteAllText("$n\src\generated\MtxUtilRuntime.cpp",$mtxRuntime,[System.Text.UTF8Encoding]::new($false))
$movementRuntime=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Util\ActorMovementUtil.cpp")
$movementAtanPattern='std::atan2\('
if([regex]::Matches($movementRuntime,$movementAtanPattern).Count -ne 2){throw 'ActorMovementUtil atan2 anchor count mismatch'}
$movementRuntime=[regex]::Replace($movementRuntime,$movementAtanPattern,'::atan2(')
$movementRuntime=ReplaceOnce $movementRuntime 'PSVECScale(&toSensorB, pOut, 1.0f / distance);' '*pOut = toSensorB * (1.0f / distance);' 'ActorMovementUtil sensor direction normalization'
$movementRuntime=ReplaceOnce $movementRuntime 'stack_8.getEuler(pActor->mRotation);' "TPos3f quatMatrix;`r`n        quatMatrix.setQuat(stack_8);`r`n        quatMatrix.getEuler(pActor->mRotation);" 'ActorMovementUtil quaternion Euler conversion'
$movementDuplicateLines=@(102,121,127,133,139,147,151,375,380,385)
$movementRuntime=RemoveCxxDefinitionsAtLines $movementRuntime $movementDuplicateLines 'ActorMovementUtil duplicate provider pruning'
[System.IO.File]::WriteAllText("$n\src\generated\ActorMovementUtilRuntime.cpp",$movementRuntime,[System.Text.UTF8Encoding]::new($false))
