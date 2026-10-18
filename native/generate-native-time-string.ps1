$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\StringUtil.cpp" -Raw
$a=$s.IndexOf('    void makeClearTimeString(');$b=$s.IndexOf('    wchar_t* addNumberFontTag(', $a)
$c=$s.IndexOf('    void makeRaceBestTimeString(');$d=$s.IndexOf('    void copyString(', $c)
if($a -lt 0 -or $b -lt 0 -or $c -lt 0 -or $d -lt 0){throw 'Original time formatting functions missing'}
$prefix=@"
#include "Game/Util/StringUtil.hpp"
#include "Game/Screen/ReplaceTagProcessor.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#define CENTISEC_PER_SEC 100
#define SEC_PER_MIN 60
#define FRAME_PER_SEC 60
#define FRAME_PER_MIN 3600
#define FRAME_PER_HOUR 216000
namespace MR {
"@
($prefix+"`r`n"+$s.Substring($a,$b-$a)+$s.Substring($c,$d-$c)+"`r`n}")|Set-Content "$n\src\generated\TimeStringKernel.cpp"
