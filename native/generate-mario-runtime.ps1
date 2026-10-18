$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
$petariRoot=Join-Path $projectRoot 'Petari'
$headerSource=Get-Content -LiteralPath (Join-Path $petariRoot 'include\Game\Player\Mario.hpp') -Raw -Encoding UTF8
$jumpSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioJump.cpp') -Raw -Encoding UTF8
$field='/* 0xA38 */ u32 _A38;'
$cast='reinterpret_cast< u32 >(pSensor)'
if(([regex]::Matches($headerSource,[regex]::Escape($field))).Count -ne 1){throw 'Mario pointer field anchor changed'}
if(([regex]::Matches($jumpSource,[regex]::Escape($cast))).Count -ne 1){throw 'MarioJump pointer cast anchor changed'}
$headerSource="// Generated native adaptation; regenerate from the pinned Petari source.`r`n"+$headerSource.Replace($field,'/* Native pointer-sized tornado sensor; original offset is not host ABI. */ std::uintptr_t _A38;')
$jumpSource="// Generated native adaptation; only tornado sensor pointer storage is widened.`r`n"+$jumpSource.Replace($cast,'reinterpret_cast< std::uintptr_t >(pSensor)')
$generatedRoot=Join-Path $PSScriptRoot 'src\generated'
New-Item -ItemType Directory -Force -Path $generatedRoot | Out-Null
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $PSScriptRoot 'include\Game\Player\Mario.hpp') -Value $headerSource
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioJump.cpp') -Value $jumpSource
# Original matching declarations collide under a standard C++ compiler.
# Use namespace aliases to the same const defaults rather than create duplicate data.
$tevsSource=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\JSystem\include\JSystem\J3DGraphBase\J3DTevs.hpp') -Raw -Encoding UTF8
$pattern='(?s)// doing this matches\. figure out why\r?\nnamespace J3DTevsDefault \{.*?\}  // namespace J3DTevsDefault'
if(([regex]::Matches($tevsSource,$pattern)).Count -ne 1){throw 'J3D default linkage anchor changed'}
$tevsSource=[regex]::Replace($tevsSource,$pattern,'// Native aliases below share the original global default data.')
$aliasAnchor='extern const J3DTevSwapModeInfo j3dDefaultTevSwapMode;'
if(([regex]::Matches($tevsSource,[regex]::Escape($aliasAnchor))).Count -ne 1){throw 'J3D swap default anchor changed'}
$tevsSource=$tevsSource.Replace($aliasAnchor,$aliasAnchor+"`r`nnamespace J3DTevsDefault { using ::j3dDefaultIndTevStageInfo; using ::j3dDefaultTevStageInfo; using ::j3dDefaultTevSwapMode; }")
$tevsTarget=Join-Path $PSScriptRoot 'include\JSystem\J3DGraphBase'
foreach($offset in @('0','4')) {
    $anchor="J3DGDWriteBPCmd(*(u32*)&field_0x$offset);"
    if(([regex]::Matches($tevsSource,[regex]::Escape($anchor))).Count -ne 1){throw 'J3D TEV packed command anchor changed'}
    $replacement="{ const u8* bytes = &field_0x$offset; J3DGDWriteBPCmd(u32(bytes[0]) << 24 | u32(bytes[1]) << 16 | u32(bytes[2]) << 8 | bytes[3]); }"
    $tevsSource=$tevsSource.Replace($anchor,$replacement)
}
New-Item -ItemType Directory -Force -Path $tevsTarget | Out-Null
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $tevsTarget 'J3DTevs.hpp') -Value ("// Generated native linkage adaptation; pinned source remains unchanged.`r`n"+$tevsSource)
$marioSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\Mario.cpp') -Raw -Encoding UTF8
$frontCheck='MR::normalizeOrZero(&front) == nullptr'
$headCheck='MR::normalizeOrZero(&headVec) != nullptr'
if(([regex]::Matches($marioSource,[regex]::Escape($frontCheck))).Count -ne 1 -or ([regex]::Matches($marioSource,[regex]::Escape($headCheck))).Count -ne 1){throw 'Mario boolean comparison anchors changed'}
$marioSource=$marioSource.Replace($frontCheck,'MR::normalizeOrZero(&front) == false').Replace($headCheck,'MR::normalizeOrZero(&headVec) != false')
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'Mario.cpp') -Value ("// Generated native adaptation of invalid bool/nullptr comparisons; pinned source unchanged.`r`n"+$marioSource)
$listSource=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\nw4r\include\nw4r\ut\LinkList.h') -Raw -Encoding UTF8
if(([regex]::Matches($listSource,'friend class Self;')).Count -ne 1){throw 'NW4R list friend anchor changed'}
$listSource=$listSource.Replace('friend class Self;','friend Self;')
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $PSScriptRoot 'include\nw4r\ut\LinkList.h') -Value ("// Generated native friend-type adaptation; pointer types come from native misc.h.`r`n"+$listSource)
$heapSource=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\RVL_SDK\include\revolution\mem\heapCommon.h') -Raw -Encoding UTF8
if(([regex]::Matches($heapSource,'typedef u32 UIntPtr;')).Count -ne 1){throw 'SDK heap pointer alias changed'}
$heapSource=$heapSource.Replace('typedef u32 UIntPtr;','typedef std::uintptr_t UIntPtr;')
$heapTarget=Join-Path $PSScriptRoot 'include\revolution\mem'
New-Item -ItemType Directory -Force -Path $heapTarget | Out-Null
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $heapTarget 'heapCommon.h') -Value ("// Generated native pointer-width adaptation; offset APIs still use original u32 domain.`r`n"+$heapSource)
$ripperSource=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\JSystem\include\JSystem\JKernel\JKRDvdRipper.hpp') -Raw -Encoding UTF8
if(([regex]::Matches($ripperSource,'static void\* loadToMainRAM\(')).Count -ne 3){throw 'DVD ripper overload declarations changed'}
$ripperSource=$ripperSource.Replace('unsigned long','u32')
$ripperSource=[regex]::Replace($ripperSource,'\blong\b','s32')
$ripperTarget=Join-Path $PSScriptRoot 'include\JSystem\JKernel'
New-Item -ItemType Directory -Force -Path $ripperTarget | Out-Null
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $ripperTarget 'JKRDvdRipper.hpp') -Value ("// Generated native fixed-width DVD API declarations; loader backend remains unported.`r`n"+$ripperSource)
# Compile the retail runtime against its full actor declaration, not the diagnostic sensor shell.
$runtimeActorTarget=Join-Path $PSScriptRoot 'include-runtime\Game\LiveActor'
New-Item -ItemType Directory -Force -Path $runtimeActorTarget | Out-Null
$runtimeActorSource=Get-Content -LiteralPath (Join-Path $petariRoot 'include\Game\LiveActor\LiveActor.hpp') -Raw -Encoding UTF8
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $runtimeActorTarget 'LiveActor.hpp') -Value ("// Full pinned retail actor declaration for native runtime object compilation.`r`n"+$runtimeActorSource)
$runtimeUtilTarget=Join-Path $PSScriptRoot 'include-runtime\Game\Util'
New-Item -ItemType Directory -Force -Path $runtimeUtilTarget | Out-Null
$runtimeUtilSource=Get-Content -LiteralPath (Join-Path $petariRoot 'include\Game\Util\LiveActorUtil.hpp') -Raw -Encoding UTF8
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $runtimeUtilTarget 'LiveActorUtil.hpp') -Value ("// Full pinned utility API; implementations remain required before runtime linking.`r`n"+$runtimeUtilSource)
$actorSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioActor.cpp') -Raw -Encoding UTF8
$actorSource=$actorSource.Replace("`r`n","`n")
$anchors=@(
    @("case 1:`n        bool didSpinPunch", "case 1: {`n        bool didSpinPunch"),
    @("        if (didSpinPunch) {`n            _946 = mConst->getTable()->mSpinIntervalTime + 0x22;`n        }`n`n        break;`n    case 2:", "        if (didSpinPunch) {`n            _946 = mConst->getTable()->mSpinIntervalTime + 0x22;`n        }`n`n        break;`n    }`n    case 2:"),
    @("case 4:`n        if (mMario->_418 != 0)", "case 4: {`n        if (mMario->_418 != 0)"),
    @("        _946 = pConstants->mTeresaWallThroughTime + pConstants->mSpinIntervalTime;`n        break;`n    case 5:", "        _946 = pConstants->mTeresaWallThroughTime + pConstants->mSpinIntervalTime;`n        break;`n    }`n    case 5:")
)
foreach($anchor in $anchors){if(([regex]::Matches($actorSource,[regex]::Escape($anchor[0]))).Count -ne 1){throw 'MarioActor case-scope anchor changed'};$actorSource=$actorSource.Replace($anchor[0],$anchor[1])}
# Preserve the retail 32-byte texture-buffer alignment using standard host allocation.
foreach($field in @('_1D8','_1DC')) {
    $old="$field = new (0x20) FBO[MR::getFrameBufferWidth()];"
    if(([regex]::Matches($actorSource,[regex]::Escape($old))).Count -ne 1){throw 'MarioActor aligned buffer anchor changed'}
    $actorSource=$actorSource.Replace($old,"$field = new (std::align_val_t{0x20}) FBO[MR::getFrameBufferWidth()];")
}
$actorSource="#include <new>`n"+$actorSource
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioActor.cpp') -Value ("// Generated native switch-scope adaptation; retail action statements preserved.`n"+$actorSource)


# Console unsigned long was 32-bit; explicitly select the integer frame-count overload.
$moveSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioMove.cpp') -Raw -Encoding UTF8
$moveAnchor='setFrontVecKeepUp(direction, 15UL);'
if(([regex]::Matches($moveSource,[regex]::Escape($moveAnchor))).Count -ne 1){throw 'MarioMove integer overload anchor changed'}
$moveSource=$moveSource.Replace($moveAnchor,'setFrontVecKeepUp(direction, u32{15});')
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioMove.cpp') -Value ("// Generated native fixed-width overload selection; original frame count retained.`r`n"+$moveSource)
$gravitySource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioActorGravity.cpp') -Raw -Encoding UTF8
$gravityAnchor='setFrontVecKeepUp(head, 1UL);'
if(([regex]::Matches($gravitySource,[regex]::Escape($gravityAnchor))).Count -ne 1){throw 'MarioActorGravity integer overload anchor changed'}
$gravitySource=$gravitySource.Replace($gravityAnchor,'setFrontVecKeepUp(head, u32{1});')
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioActorGravity.cpp') -Value ("// Generated native fixed-width overload selection; original frame count retained.`r`n"+$gravitySource)
$collisionSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioCollision.cpp') -Raw -Encoding UTF8
$collisionAnchor='startPadVib(0UL);'
if(([regex]::Matches($collisionSource,[regex]::Escape($collisionAnchor))).Count -ne 1){throw 'MarioCollision numeric vibration anchor changed'}
$collisionSource=$collisionSource.Replace($collisionAnchor,'startPadVib(u32{0});')
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioCollision.cpp') -Value ("// Generated native integer vibration ID overload; zero is not a null string.`r`n"+$collisionSource)
# Keep the original trig-table cell layout out of the host standard-library namespace.
$trigSource=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\JSystem\include\JSystem\JMath\JMATrigonometric.hpp') -Raw -Encoding UTF8
foreach($anchor in @('namespace std {','struct pair {','pair() {','std::pair< T, T > table[LEN];')) {
    if(([regex]::Matches($trigSource,[regex]::Escape($anchor))).Count -ne 1){throw 'JMath trig pair anchor changed'}
}
$trigSource=$trigSource.Replace('namespace std {','namespace JMath {').Replace('struct pair {','struct TrigTablePair {').Replace('pair() {','TrigTablePair() {').Replace('}  // namespace std','}  // namespace JMath').Replace('std::pair< T, T > table[LEN];','TrigTablePair< T, T > table[LEN];')
$trigTarget=Join-Path $PSScriptRoot 'include\JSystem\JMath'
New-Item -ItemType Directory -Force -Path $trigTarget | Out-Null
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $trigTarget 'JMATrigonometric.hpp') -Value ("// Generated native trig-cell namespace adaptation; table fields and operations preserved.`r`n"+$trigSource)
# Retail units need the full SDK umbrella, not the diagnostic matrix-only umbrella.
# Preload native types outside SDK extern-C blocks because it includes host C++ headers.
$umbrellaSource=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\RVL_SDK\include\revolution.h') -Raw -Encoding UTF8
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $PSScriptRoot 'include-runtime\revolution.h') -Value ("// Full pinned SDK declarations for runtime probes; native SDK services still require implementations.`r`n#include <revolution/types.h>`r`n"+$umbrellaSource)
$effectSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioEffect.cpp') -Raw -Encoding UTF8
$addAnchor='_BA4->add(cMaterialEffectTable[i].mName, reinterpret_cast< u32 >(&cMaterialEffectTable[i]), false);'
$searchAnchor='_BA4->search(pName, reinterpret_cast< u32* >(&entry));'
if(([regex]::Matches($effectSource,[regex]::Escape($addAnchor))).Count -ne 1 -or ([regex]::Matches($effectSource,[regex]::Escape($searchAnchor))).Count -ne 2){throw 'MarioEffect pointer lookup anchors changed'}
$effectSource=$effectSource.Replace($addAnchor,'_BA4->addPointer(cMaterialEffectTable[i].mName, &cMaterialEffectTable[i], false);')
$effectSource=$effectSource.Replace($searchAnchor,"void* nativeEntry = nullptr;`r`n    _BA4->searchPointer(pName, &nativeEntry);`r`n    entry = static_cast<MaterialEffectEntry*>(nativeEntry);")
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioEffect.cpp') -Value ("// Generated native pointer-safe material lookup; pinned source unchanged.`r`n"+$effectSource)
# Exact native source edits for original typed integer literals and invalid null tests.
$controllerEdits=@(
 @('MarioBee','setFrontVecKeepUp(front, 1UL);','setFrontVecKeepUp(front, u32{1});',1),
 @('MarioDamage','startPadVib(0UL);','startPadVib(u32{0});',1),
 @('MarioStick','startPadVib(0ul);','startPadVib(u32{0});',1),
 @('MarioWall','startPadVib(0UL);','startPadVib(u32{0});',1),
 @('MarioActorSensor','startPadVib(0UL);','startPadVib(u32{0});',1),
 @('MarioActorEye','MR::getRandom(60L, 360L)','MR::getRandom(s32{60}, s32{360})',2),
 @('MarioTeresa','MR::getRandom(60L, 180L)','MR::getRandom(s32{60}, s32{180})',2),
 @('MarioTeresa','MR::getRandom(90L, 240L)','MR::getRandom(s32{90}, s32{240})',1),
 @('MarioSideStep','changeAnimation("壁押し", 0UL);','changeAnimation("壁押し", u32{0});',2),
 @('MarioActorRushMsg','if (out != nullptr)','if (out)',1),
 @('MarioActorParts','if (_468 == nullptr)','if (_468 == 0)',1)
)
foreach($unit in @($controllerEdits | ForEach-Object { $_[0] } | Select-Object -Unique)) {
 $source=Get-Content -LiteralPath (Join-Path $petariRoot "src\Game\Player\$unit.cpp") -Raw -Encoding UTF8
 foreach($edit in $controllerEdits){if($edit[0] -ne $unit){continue};if(([regex]::Matches($source,[regex]::Escape($edit[1]))).Count -ne $edit[3]){throw "Controller edit anchor changed: $unit"};$source=$source.Replace($edit[1],$edit[2])}
 Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot "$unit.cpp") -Value ("// Generated native fixed-width overload/null-test adaptation; values and branches retained.`r`n"+$source)
}
$swimSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioSwim.cpp') -Raw -Encoding UTF8
if(([regex]::Matches($swimSource,'DEG_TO_RAD\(')).Count -ne 8){throw 'MarioSwim angle conversion sites changed'}
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioSwim.cpp') -Value ("// Native equivalent of pinned MSL angle conversion macro.`r`n#include <Native/AngleConversion.hpp>`r`n"+$swimSource)
$warpSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioWarp.cpp') -Raw -Encoding UTF8
$warpSource=$warpSource.Replace("`r`n","`n")
$warpEdits=@(
 @('    TVec3< f32 > TVec3< f32 >::operator*(f32) const NO_INLINE;','    // Native generic vector scalar operator is already defined.'),
 @("    case 3:`n        MR::endGlobalEventCamera(`"引き戻し`", -1, true);","    case 3: {`n        MR::endGlobalEventCamera(`"引き戻し`", -1, true);"),
 @("        player->_430 = 0;`n        break;`n    case 1:","        player->_430 = 0;`n        break;`n    }`n    case 1:")
)
foreach($edit in $warpEdits){if(([regex]::Matches($warpSource,[regex]::Escape($edit[0]))).Count -ne 1){throw 'MarioWarp native scope anchor changed'};$warpSource=$warpSource.Replace($edit[0],$edit[1])}
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioWarp.cpp') -Value ("// Generated native matching-declaration removal and case scope; warp behavior retained.`n"+$warpSource)
$shadowSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioShadow.cpp') -Raw -Encoding UTF8
foreach($anchor in @('new (32) TVec3f[_58 * _5A]','new (32) TVec2f[_58 * _5A]','new (32) u8[size]')) {
 if(([regex]::Matches($shadowSource,[regex]::Escape($anchor))).Count -ne 1){throw 'MarioShadow allocation anchor changed'}
 $shadowSource=$shadowSource.Replace($anchor,$anchor.Replace('new (32)','new (std::align_val_t{32})'))
}
if(([regex]::Matches($shadowSource,'register f32 texScale')).Count -ne 1){throw 'MarioShadow register anchor changed'}
$shadowSource=$shadowSource.Replace('register f32 texScale','f32 texScale')
$shadowWriterAnchor='void CollisionShadow::createDL() {'
if(!$shadowSource.Contains($shadowWriterAnchor)){throw 'Collision shadow writer scope anchor changed.'}
$shadowSource='#include <Native/GDCurrentScope.hpp>'+"`n"+$shadowSource.Replace($shadowWriterAnchor,$shadowWriterAnchor+"`n    Native::GDCurrentScope restoreWriter;")
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioShadow.cpp') -Value ("// Native aligned allocation and obsolete register syntax adaptation.`r`n#include <new>`r`n"+$shadowSource)
$searchSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\Game\Player\MarioSearchLight.cpp') -Raw -Encoding UTF8
$searchAnchor='new (0x20) Mtx[num]'
if(([regex]::Matches($searchSource,[regex]::Escape($searchAnchor))).Count -ne 1){throw 'Searchlight aligned matrix anchor changed'}
$searchSource=$searchSource.Replace($searchAnchor,'new (std::align_val_t{32}) Mtx[num]')
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'MarioSearchLight.cpp') -Value ("// Generated native aligned matrix allocation; rendering statements retained.`r`n#include <new>`r`n"+$searchSource)
foreach($drawUnit in @('MarioActorDraw','MarioActorSpecialDraw')) {
 $drawSource=Get-Content -LiteralPath (Join-Path $petariRoot "src\Game\Player\$drawUnit.cpp") -Raw -Encoding UTF8
 $drawPattern=if($drawUnit -eq 'MarioActorDraw'){'new \(0x20\)'}else{'new \(32\)'}
 $expected=if($drawUnit -eq 'MarioActorDraw'){5}else{1}
 if(([regex]::Matches($drawSource,$drawPattern)).Count -ne $expected){throw 'Original draw allocation sites changed'}
 $drawSource=[regex]::Replace($drawSource,$drawPattern,'new (std::align_val_t{32})')
 if($drawUnit -eq 'MarioActorSpecialDraw') {
  $randomAnchor='MR::getRandom(0L, 8L)'
  if(([regex]::Matches($drawSource,[regex]::Escape($randomAnchor))).Count -ne 2){throw 'Special draw random sites changed'}
  $drawSource=$drawSource.Replace($randomAnchor,'MR::getRandom(s32{0}, s32{8})')
 }
 $drawSource=& (Join-Path $PSScriptRoot 'adapt-player-private-writers.ps1') -SourceText $drawSource -Unit $drawUnit
 Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot "$drawUnit.cpp") -Value ("// Native aligned drawing arrays and original integer random ranges.`r`n#include <new>`r`n"+$drawSource)
}
$audioHeapSource=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\JSystem\include\JSystem\JAudio2\JASHeapCtrl.hpp') -Raw -Encoding UTF8
foreach($anchor in @('static void* operator new(u32 size)','static void operator delete(void* addr, u32 size)','return memPool_.alloc(size);','memPool_.free(addr, size);')) {
 if(([regex]::Matches($audioHeapSource,[regex]::Escape($anchor))).Count -ne 2){throw 'Audio pool allocation signature anchors changed'}
}
$audioHeapSource=$audioHeapSource.Replace('static void* operator new(u32 size)','static void* operator new(std::size_t size)').Replace('static void operator delete(void* addr, u32 size)','static void operator delete(void* addr, std::size_t size)').Replace('return memPool_.alloc(size);','return memPool_.alloc(nativeAudioPoolSize(size));').Replace('memPool_.free(addr, size);','memPool_.free(addr, static_cast<u32>(size));')
$audioHeapSource=$audioHeapSource.Replace('new (JASKernel::getSystemHeap(), 0) MemoryChunk','new (JASKernel::getSystemHeap(), alignof(MemoryChunk)) MemoryChunk').Replace('new (JKRHeap::getSystemHeap(), 0) MemoryChunk','new (JKRHeap::getSystemHeap(), alignof(MemoryChunk)) MemoryChunk').Replace('delete chunk;','chunk->~MemoryChunk(); JKRHeap::free(chunk, nullptr);')
$audioHeapSource=$audioHeapSource.Replace('return (u8*)this + 0xc <= (u8*)ptr && (u8*)ptr < (u8*)this + (0xc + ChunkSize);','return mBuffer <= static_cast<const u8*>(ptr) && static_cast<const u8*>(ptr) < mBuffer + ChunkSize;').Replace('u8 mBuffer[ChunkSize];','alignas(std::max_align_t) u8 mBuffer[ChunkSize];').Replace('u32 freeSize = mChunk->getFreeSize();','if(size > ChunkSize) return nullptr; size = (size + alignof(std::max_align_t)-1) & ~(u32(alignof(std::max_align_t))-1); if(size > ChunkSize) return nullptr; u32 freeSize = mChunk->getFreeSize();')
$audioHeapTarget=Join-Path $PSScriptRoot 'include\JSystem\JAudio2'
New-Item -ItemType Directory -Force -Path $audioHeapTarget | Out-Null
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $audioHeapTarget 'JASHeapCtrl.hpp') -Value ("// Generated native allocation size signatures; original audio pool backend retained.`r`n#include <Native/AudioPoolSize.hpp>`r`n"+$audioHeapSource)
$gadgetListSource=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\JSystem\include\JSystem\JGadget\linklist.hpp') -Raw -Encoding UTF8
$iteratorPattern='(?s)namespace std \{.*?\}  // namespace std'
if(([regex]::Matches($gadgetListSource,$iteratorPattern)).Count -ne 1){throw 'JGadget custom standard iterator block changed'}
$gadgetListSource=[regex]::Replace($gadgetListSource,$iteratorPattern,'// Native standard iterator tags and empty metadata base come from <iterator>.')
$gadgetTarget=Join-Path $PSScriptRoot 'include\JSystem\JGadget'
New-Item -ItemType Directory -Force -Path $gadgetTarget | Out-Null
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $gadgetTarget 'linklist.hpp') -Value ("// Native standard-library iterator coexistence; original intrusive operations retained.`r`n#include <iterator>`r`n"+$gadgetListSource)
$pointerArraySource=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\JSystem\include\JSystem\JAudio2\JASGadget.hpp') -Raw -Encoding UTF8
$pointerArrayAnchor='JASPtrArray() : JASPtrTable(mPtrArray, LEN)'
if(([regex]::Matches($pointerArraySource,[regex]::Escape($pointerArrayAnchor))).Count -ne 1){throw 'JAS pointer-array base constructor changed'}
$pointerArraySource=$pointerArraySource.Replace($pointerArrayAnchor,'JASPtrArray() : JASPtrTable< T >(mPtrArray, LEN)')
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $audioHeapTarget 'JASGadget.hpp') -Value ("// Native dependent-base template spelling; pointer table storage and behavior retained.`r`n"+$pointerArraySource)
$seqHeader=Get-Content -LiteralPath (Join-Path $petariRoot 'libs\JSystem\include\JSystem\JAudio2\JASSeqReader.hpp') -Raw -Encoding UTF8
$seqOffset='return (u32)mSeqCursor - (u32)mSeqBuff;'
if(([regex]::Matches($seqHeader,[regex]::Escape($seqOffset))).Count -ne 1){throw 'Sequence offset anchor changed'}
$seqHeader=$seqHeader.Replace($seqOffset,'return NativeSequence::offset(mSeqBuff, mSeqCursor);')
$seqEdits=@(
 @('(?s)    u32 get24\(u32 param_0\) const \{.*?\r?\n    \}',"    u32 get24(u32 param_0) const { return NativeSequence::read24(mSeqBuff + param_0); }"),
 @('(?s)    u16 get16\(u32 param_0\) const \{.*?\r?\n    \}',"    u16 get16(u32 param_0) const { return NativeSequence::read16(mSeqBuff + param_0); }"),
 @('(?s)    u32 get32\(u32 param_0\) const \{.*?\r?\n    \}',"    u32 get32(u32 param_0) const { return NativeSequence::read32(mSeqBuff + param_0); }"),
 @('(?s)    u32 read16\(\) \{.*?\r?\n    \}',"    u32 read16() { const u32 value=NativeSequence::read16(mSeqCursor); mSeqCursor+=2; return value; }"),
 @('(?s)    u32 read24\(\) \{.*?\r?\n    \}',"    u32 read24() { const u32 value=NativeSequence::read24(mSeqCursor); mSeqCursor+=3; return value; }")
)
foreach($edit in $seqEdits){if(([regex]::Matches($seqHeader,$edit[0])).Count -ne 1){throw 'Sequence read function anchor changed'};$seqHeader=[regex]::Replace($seqHeader,$edit[0],$edit[1])}
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $audioHeapTarget 'JASSeqReader.hpp') -Value ("// Native pointer-width offset and big-endian byte reads; original sequence fields/API retained.`r`n#include <Native/SequenceRead.hpp>`r`n"+$seqHeader)
$seqSource=Get-Content -LiteralPath (Join-Path $petariRoot 'src\JSystem\JAudio2\JASSeqReader.cpp') -Raw -Encoding UTF8
$seqInit='mLoopCounts[i] = nullptr;'
if(([regex]::Matches($seqSource,[regex]::Escape($seqInit))).Count -ne 1){throw 'Sequence counter initialization anchor changed'}
Set-Content -Encoding UTF8 -LiteralPath (Join-Path $generatedRoot 'JASSeqReader.cpp') -Value ("// Native numeric zero initialization; original sequence call/loop/MIDI methods retained.`r`n"+$seqSource.Replace($seqInit,'mLoopCounts[i] = 0;'))


