$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\StringUtil.cpp" -Raw
$start=$s.IndexOf('    bool isMessageEditorNextTag(');$end=$s.IndexOf('    void scan32(', $start)
if($start -lt 0 -or $end -lt 0){throw 'Original message string functions missing'}
$body=$s.Substring($start,$end-$start).Replace('wchar_t v2 = pTag->mBuffer[0];','const auto* bytes = reinterpret_cast<const u8*>(pTag); u16 v2 = (u16(bytes[4]) << 8) | bytes[5];')
$prefix=@"
#include "Game/Util/StringUtil.hpp"
namespace MR {
struct Tag {u16 marker;u8 mDataSize;u8 _3;wchar_t mBuffer[1];};
"@
($prefix+"`r`n"+$body+"`r`n}`r`n")|Set-Content "$n\src\generated\MessageStringKernel.cpp"
