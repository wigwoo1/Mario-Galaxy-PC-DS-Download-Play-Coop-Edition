$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-jpa-emitter-header.ps1"
$s=Get-Content "$p\Petari\src\JSystem\JParticle\JPAEmitterManager.cpp" -Raw
Set-Content "$n\src\generated\JPAEmitterManager.cpp" ($s.Replace('#include "JSystem/JKernel/JKRHeap.hpp"','#include <Native/HeapPlacement.hpp>'))
$s=Get-Content "$p\Petari\src\JSystem\JParticle\JPAMath.cpp" -Raw
$anchor='const std::pair< f32, f32 >* p ='
if([regex]::Matches($s,[regex]::Escape($anchor)).Count -ne 2){throw 'Original trig pair anchors changed'}
$s=$s.Replace($anchor,'const auto* p =')
Set-Content "$n\src\generated\JPAMath.cpp" ('#include <JSystem/JMath/JMath.hpp>'+"`n"+$s)

