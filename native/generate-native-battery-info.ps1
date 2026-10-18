$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Screen\BatteryInfo.cpp" -Raw
# Anonymous-namespace nerve singletons already have internal linkage. Clang
# rejects weak internal definitions; retain their original internal linkage.
$s=$s.Replace('#define WPAD_BATTERY_LEVEL_EMPTY -1', "#undef ATTRIBUTE_WEAK`r`n#define ATTRIBUTE_WEAK`r`n#define WPAD_BATTERY_LEVEL_EMPTY -1")
Set-Content "$n\src\generated\BatteryInfo.cpp" $s
