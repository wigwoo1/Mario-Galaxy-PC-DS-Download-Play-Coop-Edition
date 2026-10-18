#include <Game/System/Language.hpp>
#include <revolution/sc.h>
#define REGION_EU 0
#define REGION_JP 1
#define REGION_US 2
#define REGION_CN 3
#define REGION_KR 4
#define REGION_SHIFT 0x4
#define REGION_SHIFT_EU (REGION_EU << REGION_SHIFT)
#define REGION_SHIFT_JP (REGION_JP << REGION_SHIFT)
#define REGION_SHIFT_US (REGION_US << REGION_SHIFT)
#define REGION_SHIFT_CN (REGION_CN << REGION_SHIFT)
#define REGION_SHIFT_KR (REGION_KR << REGION_SHIFT)
#define REGION_MASK (0xF << REGION_SHIFT)

#define LANGUAGE_JPJAPANESE (REGION_SHIFT_JP | SC_LANG_JAPANESE)
#define LANGUAGE_USENGLISH (REGION_SHIFT_US | SC_LANG_ENGLISH)
#define LANGUAGE_USSPANISH (REGION_SHIFT_US | SC_LANG_SPANISH)
#define LANGUAGE_USFRENCH (REGION_SHIFT_US | SC_LANG_FRENCH)
#define LANGUAGE_EUENGLISH (REGION_SHIFT_EU | SC_LANG_ENGLISH)
#define LANGUAGE_EUSPANISH (REGION_SHIFT_EU | SC_LANG_SPANISH)
#define LANGUAGE_EUFRENCH (REGION_SHIFT_EU | SC_LANG_FRENCH)
#define LANGUAGE_EUGERMAN (REGION_SHIFT_EU | SC_LANG_GERMAN)
#define LANGUAGE_EUITALIAN (REGION_SHIFT_EU | SC_LANG_ITALIAN)
#define LANGUAGE_EUDUTCH (REGION_SHIFT_EU | SC_LANG_DUTCH)
#define LANGUAGE_CNSIMPCHINESE (REGION_SHIFT_CN | SC_LANG_SIMP_CHINESE)
#define LANGUAGE_KRKOREAN (REGION_SHIFT_KR | SC_LANG_KOREAN)
#define LANGUAGE_MASK (~REGION_MASK)

namespace {
    const u8 cSCLanguage2GameLanguageTable[][10] = {
        {
            // RMGJ
            LANGUAGE_JPJAPANESE,
            LANGUAGE_USENGLISH,
            LANGUAGE_EUGERMAN,
            LANGUAGE_EUFRENCH,
            LANGUAGE_EUSPANISH,
            LANGUAGE_EUITALIAN,
            LANGUAGE_EUDUTCH,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_KRKOREAN,
        },
        {
            // RMGE
            LANGUAGE_JPJAPANESE,
            LANGUAGE_USENGLISH,
            LANGUAGE_EUGERMAN,
            LANGUAGE_USFRENCH,
            LANGUAGE_USSPANISH,
            LANGUAGE_EUITALIAN,
            LANGUAGE_EUDUTCH,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_KRKOREAN,
        },
        {
            // RMGP
            LANGUAGE_JPJAPANESE,
            LANGUAGE_EUENGLISH,
            LANGUAGE_EUGERMAN,
            LANGUAGE_EUFRENCH,
            LANGUAGE_EUSPANISH,
            LANGUAGE_EUITALIAN,
            LANGUAGE_EUDUTCH,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_KRKOREAN,
        },
        {
            // ???
            LANGUAGE_JPJAPANESE,
            LANGUAGE_USENGLISH,
            LANGUAGE_EUGERMAN,
            LANGUAGE_EUFRENCH,
            LANGUAGE_EUSPANISH,
            LANGUAGE_EUITALIAN,
            LANGUAGE_EUDUTCH,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_KRKOREAN,
        },
        {
            // RMGK
            LANGUAGE_JPJAPANESE,
            LANGUAGE_USENGLISH,
            LANGUAGE_EUGERMAN,
            LANGUAGE_EUFRENCH,
            LANGUAGE_EUSPANISH,
            LANGUAGE_EUITALIAN,
            LANGUAGE_EUDUTCH,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_KRKOREAN,
        },
        {
            // ???
            LANGUAGE_JPJAPANESE,
            LANGUAGE_USENGLISH,
            LANGUAGE_EUGERMAN,
            LANGUAGE_EUFRENCH,
            LANGUAGE_EUSPANISH,
            LANGUAGE_EUITALIAN,
            LANGUAGE_EUDUTCH,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_KRKOREAN,
        },
    };
    const Language cLanguages[] = {
        {LANGUAGE_JPJAPANESE, "JpJapanese"}, {LANGUAGE_USENGLISH, "UsEnglish"},         {LANGUAGE_USSPANISH, "UsSpanish"},
        {LANGUAGE_USFRENCH, "UsFrench"},     {LANGUAGE_EUENGLISH, "EuEnglish"},         {LANGUAGE_EUSPANISH, "EuSpanish"},
        {LANGUAGE_EUFRENCH, "EuFrench"},     {LANGUAGE_EUGERMAN, "EuGerman"},           {LANGUAGE_EUITALIAN, "EuItalian"},
        {LANGUAGE_EUDUTCH, "EuDutch"},       {LANGUAGE_CNSIMPCHINESE, "CnSimpChinese"}, {LANGUAGE_KRKOREAN, "KrKorean"},
    };
};  // namespace

namespace MR {
    u32 getDecidedLanguageFromIPL() {
        // This port currently targets the supplied RMGE (USA/Canada) disc.
        // Unsupported console settings use the English resources present on it.
        const u32 language = SCGetLanguage();
        const u32 index = (language == SC_LANG_FRENCH || language == SC_LANG_SPANISH)
                            ? language : SC_LANG_ENGLISH;
        return ::cSCLanguage2GameLanguageTable[1][index];
    }
    u32 getLanguageNum() {
        return ARRAY_SIZE(::cLanguages);
    }

    const char* getLanguagePrefixByIndex(u32 index) {
        return ::cLanguages[index].mName;
    }
}
