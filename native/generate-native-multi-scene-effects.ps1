$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Scene\MultiSceneEffectKeeper.cpp" -Raw
$anchor='std::binder2nd< std::mem_fun1_t< void, MultiEmitter, EffectSystem* >, EffectSystem* >(std::mem_func(&MultiEmitter::forceDelete), pSystem)'
if(!$s.Contains($anchor)){throw 'Missing original force-delete adapter'}
$s=$s.Replace($anchor,'[pSystem](MultiEmitter* emitter) { emitter->forceDelete(pSystem); }').Replace('std::mem_func(', 'std::mem_fn(')
$s=$s.Replace('std::binder2nd< std::const_mem_fun1_t< bool, MultiEmitter, u16 >, u16 >(std::mem_fn(&MultiEmitter::isEqualName), hash)','[hash](const MultiEmitter* emitter) { return emitter->isEqualName(hash); }')
$s="#include <functional>`r`n"+$s
Set-Content "$n\src\generated\MultiSceneEffectKeeper.cpp" $s

