$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Effect\MultiEmitter.cpp" -Raw
$replacements=@(
@('(?s)std::for_each\(mChildren.begin\(\), mChildren.end\(\),\s*std::binder2nd.*?pCallBackBase\)\);','for (auto child : mChildren) child->createEmitterWithCallBack(pCallBackBase);'),
@('std::for_each_array\(mEmitters.begin\(\), mEmitters.end\(\), std::mem_func\(&SingleEmitter::deleteEmitter\)\);','for (auto& emitter : mEmitters) emitter.deleteEmitter();'),
@('std::for_each\(mChildren.begin\(\), mChildren.end\(\), std::mem_func\(&MultiEmitter::deleteEmitter\)\);','for (auto child : mChildren) child->deleteEmitter();'),
@('std::for_each\(mChildren.begin\(\), mChildren.end\(\), std::mem_func\(&MultiEmitter::forceDeleteEmitter\)\);','for (auto child : mChildren) child->forceDeleteEmitter();'),
@('(?s)std::for_each\(\s*mChildren.begin\(\), mChildren.end\(\),\s*std::binder2nd.*?pSystem\)\);','for (auto child : mChildren) child->forceDelete(pSystem);'),
@('std::for_each\(mChildren.begin\(\), mChildren.end\(\), std::mem_func\(&MultiEmitter::deleteForeverEmitter\)\);','for (auto child : mChildren) child->deleteForeverEmitter();'),
@('(?s)std::for_each_array\(mEmitters.begin\(\), mEmitters.end\(\),\s*std::binder2nd.*?pSystem\)\);','for (auto& emitter : mEmitters) emitter.scanParticleEmitter(pSystem);'),
@('(?s)std::for_each_array\(\s*mEmitters.begin\(\), mEmitters.end\(\),\s*std::binder2nd.*?static_cast< u8 >\(index\)\)\);','for (auto& emitter : mEmitters) emitter.setGroupID(static_cast<u8>(index));')
)
foreach($r in $replacements){if([regex]::Matches($s,$r[0]).Count -ne 1){throw "Original STL adaptation anchor count: $($r[1])"};$s=[regex]::Replace($s,$r[0],$r[1])}
Set-Content "$n\src\generated\MultiEmitter.cpp" $s
