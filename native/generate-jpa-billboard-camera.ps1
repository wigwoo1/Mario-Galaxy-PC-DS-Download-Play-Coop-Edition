$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\Overwrite.cpp" -Raw
$b=$s.IndexOf('void JPAEmitterManager::calcYBBCam()');if($b -lt 0){throw 'Original billboard camera anchor missing'}
$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
$prefix='#include <JSystem/JParticle/JPAEmitterManager.hpp>'+"`n"+'#include <JSystem/JParticle/JPAEmitter.hpp>'+"`n"+'#include <Game/Util/MathUtil.hpp>'+"`n"
$camera=$s.Substring($b,$e-$b)
$b=$s.IndexOf('void JPABaseEmitter::init(JPAEmitterManager*');if($b -lt 0){throw 'Original emitter init anchor missing'}
$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
$prefix+='#include <JSystem/JParticle/JPAResource.hpp>'+"`n"+'#include <JSystem/JParticle/JPADynamicsBlock.hpp>'+"`n"+'#include <JSystem/JParticle/JPABaseShape.hpp>'+"`n"
Set-Content "$n\src\generated\JPABillboardCameraKernel.cpp" ($prefix+$camera+"`n"+$s.Substring($b,$e-$b))

