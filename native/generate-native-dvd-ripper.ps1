$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JKernel\JKRDvdRipper.cpp" -Raw
if(!$s.Contains('!JKRDvdRipper::isErrorRetry)')){throw 'Expected original DVD retry condition missing'}
$s=$s.Replace('!JKRDvdRipper::isErrorRetry)', '!JKRDvdRipper::isErrorRetry())')
$old='endPtr = dest + (header->length - fileOffset);'
$new=@"
const u32 nativeLength = (u32(src[4]) << 24) | (u32(src[5]) << 16) | (u32(src[6]) << 8) | u32(src[7]);
    if (fileOffset > nativeLength) return -1;
    const u32 nativeRemaining = nativeLength - fileOffset;
    if (nativeRemaining == 0 || maxDest == 0) { *tsPtr = 0; return 0; }
    endPtr = dest + nativeRemaining;
"@
if(!$s.Contains($old)){throw 'Expected Yaz0 length read missing'}
$s=$s.Replace($old,$new)
$s=$s.Replace('szpEnd = szpBuf + szsBufferSize;', 'if (!szpBuf) { if (inTsPtr) *inTsPtr = 0; OSUnlockMutex(&decompMutex); return -1; }' + "`r`n    szpEnd = szpBuf + szsBufferSize;")
$s=$s.Replace('refEnd = refBuf + 0x1120;', 'if (!refBuf) { JKRFree(szpBuf); if (inTsPtr) *inTsPtr = 0; OSUnlockMutex(&decompMutex); return -1; }' + "`r`n        refEnd = refBuf + 0x1120;")
$s="#ifndef IS_NOT_ALIGNED`r`n#define IS_NOT_ALIGNED(X,N) ((X)&((N)-1))`r`n#endif`r`n"+$s
$s|Set-Content "$n\src\generated\JKRDvdRipperKernel.cpp"


