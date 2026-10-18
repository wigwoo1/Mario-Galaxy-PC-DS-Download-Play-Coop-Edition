$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\Language.cpp" -Raw
$a=$s.IndexOf('    u32 getDecidedLanguageFromIPL() {')
$b=$s.IndexOf('    u32 getLanguage() {',$a)
if($a -lt 0 -or $b -lt 0){throw 'Original language function anchors missing'}
$decision=@"
    u32 getDecidedLanguageFromIPL() {
        // This port currently targets the supplied RMGE (USA/Canada) disc.
        // Unsupported console settings use the English resources present on it.
        const u32 language = SCGetLanguage();
        const u32 index = (language == SC_LANG_FRENCH || language == SC_LANG_SPANISH)
                            ? language : SC_LANG_ENGLISH;
        return ::cSCLanguage2GameLanguageTable[1][index];
    }

"@
$s=$s.Substring(0,$a)+$decision+$s.Substring($b)
Set-Content "$n\src\generated\Language.cpp" $s
# Exercise the actual adapted decision and original table/prefix lookup without
# fabricating a GameSystem singleton or linking unrelated unfinished startup.
$tableStart=$s.IndexOf('#define REGION_EU')
$tableStop=$s.IndexOf('namespace MR {',$tableStart)
$num=$s.IndexOf('    u32 getLanguageNum() {')
$end=$s.LastIndexOf('};  // namespace MR')
$fixture='#include <Game/System/Language.hpp>'+"`n"+'#include <revolution/sc.h>'+"`n"+$s.Substring($tableStart,$tableStop-$tableStart)+'namespace MR {'+"`n"+$decision+$s.Substring($num,$end-$num)+'}'
Set-Content "$n\src\generated\LanguageDecisionFixture.cpp" $fixture

