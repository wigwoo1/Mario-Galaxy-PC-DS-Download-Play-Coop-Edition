$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\StringUtil.cpp" -Raw
$out="#include `"Game/Util/StringUtil.hpp`"`r`n#include <cstdio>`r`n#include <cstring>`r`n#include <cctype>`r`n#include <revolution/types.h>`r`nnamespace MR {`r`n"
foreach($sig in @('    void scan16(', '    void scans16x4(', '    void scan8(const char* pSrc, const char* pSubStr, u8* pDst)', '    void scan32(const char* pSrc, const char* pSubStr, s32* pDst)', '    void scanf32(const char* pSrc, const char* pSubStr, f32* pDst)', '    void scanu8x4(const char* pSrc, const char* pSubStr, u8* pDst)', '    void scanf32x4(const char* pSrc, const char* pSubStr, f32* pDst)', '    bool isEqualSubString(const char* pStr, const char* pSubStr)', '    bool hasStringSpace(const char* pStr)', '    bool isDigitStringTail(const char* pStr, int digitNum)', '    bool isNullOrEmptyString(const char* pStr)')) {
$a=$s.IndexOf($sig);if($a -lt 0){throw "Missing signature: $sig"}
$b=$s.IndexOf('{',$a);$depth=1;$i=$b+1
while($depth -gt 0 -and $i -lt $s.Length){if($s[$i] -eq '{'){$depth++};if($s[$i] -eq '}'){$depth--};$i++}
if($depth -ne 0){throw 'Unbalanced source body'}
$out+=$s.Substring($a,$i-$a)+"`r`n"
}
$out+="}`r`n"
Set-Content "$n\src\generated\RuntimeStringScans.cpp" $out



