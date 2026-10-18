$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
foreach($name in @('JPAResourceManager','JPAResource','JPAResourceLoader','JPATexture','JPAKeyBlock','JPAExtraShape','JPAExTexShape','JPADynamicsBlock','JPAChildShape','JPAFieldBlock','JPABaseShape')){
$s=Get-Content "$p\Petari\src\JSystem\JParticle\$name.cpp" -Raw
$anchor='#include "JSystem/JKernel/JKRHeap.hpp"'
if($s.Contains($anchor)){$s=$s.Replace($anchor,'#include <Native/HeapPlacement.hpp>')}else{$s='#include <Native/HeapPlacement.hpp>'+"`n"+$s}
if($name -eq 'JPAExtraShape'){
$s=[regex]::Replace($s,'(?<!__)OSf32tou8\(','nativeJPAf32tou8(')
$s=$s.Replace('#include <revolution/os.h>','#include <revolution/os.h>'+"`n"+'static void nativeJPAf32tou8(f32*, u8*);')
}
if($name -eq 'JPAResource'){
 $s=[regex]::Replace($s,'\((EmitterFunc|ParticleFunc)\*\)JKRAllocFromHeap\(heap, (\w+) \* 4, 4\)', '($1*)JKRAllocFromHeap(heap, $2 * sizeof($1), alignof($1))')
}
Set-Content "$n\src\generated\$name.cpp" $s
}


