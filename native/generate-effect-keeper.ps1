$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\LiveActor\EffectKeeper.cpp" -Raw
foreach($method in @('deleteEmitter','forceDeleteEmitter','stopEmitterOnClipped','playEmitterOffClipped','playCalcAndDeleteForeverEmitter')){
$old='std::for_each(_C.begin(), _C.end(), std::mem_func(&MultiEmitter::'+$method+'));'
if(!$s.Contains($old)){throw "Original effect traversal missing: $method"};$s=$s.Replace($old,'for (auto emitter : _C) emitter->'+$method+'();')
}
foreach($method in @('playDrawParticle','stopDrawParticle')){
$pattern='(?s)std::for_each\(_C.begin\(\), _C.end\(\),\s*std::binder2nd< std::mem_fun1_t< void, MultiEmitter, s32 >, s32 >\(std::mem_func\(&MultiEmitter::'+$method+'\), -1\)\);'
if([regex]::Matches($s,$pattern).Count -ne 1){throw "Original effect bound traversal missing: $method"};$s=[regex]::Replace($s,$pattern,'for (auto emitter : _C) emitter->'+$method+'(-1);')
}
if([regex]::Matches($s,'reinterpret_cast< u32 >\(pEmitter\)').Count -ne 2){throw 'Original pointer add anchors changed'}
$s=$s.Replace('_18->add(pParam3, reinterpret_cast< u32 >(pEmitter), 0);','_18->addPointer(pParam3, pEmitter, false);').Replace('_18->add(pParam2, reinterpret_cast< u32 >(pEmitter), 0);','_18->addPointer(pParam2, pEmitter, false);')
$old='u32 result = 0;'
if(!$s.Contains($old) -or !$s.Contains('_18->search(pParam1, &result)')){throw 'Original pointer search anchor changed'}
$s=$s.Replace($old,'void* result = nullptr;').Replace('_18->search(pParam1, &result)','_18->searchPointer(pParam1, &result)').Replace('reinterpret_cast< MultiEmitter* >(result)','static_cast<MultiEmitter*>(result)')
Set-Content "$n\src\generated\EffectKeeper.cpp" $s

