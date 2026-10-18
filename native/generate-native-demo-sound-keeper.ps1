$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Demo\DemoSoundKeeper.cpp" -Raw
$old='pMap->getValue(i, "ReturnBgm", reinterpret_cast< s32* >(&info.mReturnBgm));'
if(!$s.Contains($old)){throw 'Original return BGM read missing'}
$s=$s.Replace($old,'s32 nativeReturnBgm = 0; pMap->getValue(i, "ReturnBgm", &nativeReturnBgm); info.mReturnBgm = nativeReturnBgm != 0;')
$s|Set-Content "$n\src\generated\DemoSoundKeeper.cpp"
