$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AudioLib\OverwriteJAudio.cpp" -Raw
$start=$s.IndexOf('JASInstParam::JASInstParam()');if($start -lt 0){throw 'Missing game instrument constructor'}
Set-Content "$n\src\generated\GameInstrumentParamKernel.cpp" ('#include <JSystem/JAudio2/JASBasicInst.hpp>'+[Environment]::NewLine+'#include <JSystem/JAudio2/JASChannel.hpp>'+[Environment]::NewLine+$s.Substring($start))
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JASBasicInst.hpp" -Raw
$h=$h.Replace('JASInstParam() ATTRIBUTE_WEAK;','JASInstParam();')
Set-Content "$n\include\JSystem\JAudio2\JASBasicInst.hpp" $h
