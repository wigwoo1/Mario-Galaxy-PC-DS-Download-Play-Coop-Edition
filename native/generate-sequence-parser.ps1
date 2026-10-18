$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASSeqParser.cpp" -Raw
$s=$s.Replace('u32 values[4];','std::uintptr_t values[4]{};').Replace('u8 formatTypes[4];','u8 formatTypes[4]{};').Replace('(u32)track->getSeqCtrl()->getAddr(values[i])','reinterpret_cast<std::uintptr_t>(track->getSeqCtrl()->getAddr(values[i]))').Replace('JASReport(formatString, values[0], values[1], values[2], values[3]);','Native::reportSequence(formatString, values, formatTypes);')
Set-Content "$n\src\generated\SequenceParserKernel.cpp" ('#include <Native/SequenceReport.hpp>'+[Environment]::NewLine+$s)
