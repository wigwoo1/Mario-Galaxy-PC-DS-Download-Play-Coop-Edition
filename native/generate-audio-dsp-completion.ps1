$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASDSPInterface.cpp" -Raw
$out='#include <JSystem/JAudio2/JASDspInterface.hpp>'+[Environment]::NewLine+'#include <JSystem/JAudio2/dspproc.hpp>'+[Environment]::NewLine+'#include <JSystem/JAudio2/dsptask.hpp>'+[Environment]::NewLine
foreach($name in @('releaseHalt','finishWork')){ $match=[regex]::Match($source,'void JASDsp::'+$name+'\(');if(!$match.Success){throw "Missing $name"};$open=$source.IndexOf('{',$match.Index);$end=$source.IndexOf('}',$open)+1;$out+=$source.Substring($match.Index,$end-$match.Index)+[Environment]::NewLine }
Set-Content "$n\src\generated\AudioDSPCompletionKernel.cpp" $out
