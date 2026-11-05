$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRMemArchive.cpp")
$s=$s.Replace('unsigned long','u32').Replace('fixedInit(long entryNum)','fixedInit(std::intptr_t entryNum)').Replace('reinterpret_cast< s32 >(a1)','reinterpret_cast<std::intptr_t>(a1)').Replace('reinterpret_cast< u32 >(mHeader)','reinterpret_cast<std::uintptr_t>(mHeader)')
$s=[regex]::Replace($s,'case JKR_COMPRESSION_SZS:\r?\n        u32 size','case JKR_COMPRESSION_SZS: {'+[Environment]::NewLine+'        u32 size')
$s=[regex]::Replace($s,'return size;\r?\n    default:','return size;'+[Environment]::NewLine+'    }'+[Environment]::NewLine+'    default:')
function ReplaceBody([string]$signature,[string]$body){
 if([regex]::Matches($script:s,[regex]::Escape($signature)).Count-ne 1){throw "Memory archive method anchor: $signature"}
 $b=$script:s.IndexOf($signature);$o=$script:s.IndexOf('{',$b);$d=1;$e=$o+1
 while($d){if($e-ge $script:s.Length){throw 'Unterminated archive method'};if($script:s[$e]-eq '{'){$d++};if($script:s[$e]-eq '}'){$d--};$e++}
 $script:s=$script:s.Remove($o,$e-$o).Insert($o,$body)
}
# Existing caller-buffer mount keeps an owned, decoded host-table snapshot.
ReplaceBody 'bool JKRMemArchive::open(void* pData, u32 a2, JKRMemBreakFlag breakFlag)' @'
{
    if (!nativeAttachRarc(this, pData)) return false;
    mHeap = JKRHeap::findFromRoot(pData);
    _5C = 0;
    return true;
}
'@
# Keep original DVD ripper, compression and heap-direction policies; decode
# on-disk big-endian tables through the bounded owner used by buffer mounts.
ReplaceBody 'bool JKRMemArchive::open(long entryNum, EMountDirection mountDir)' @'
{
    mHeader = nullptr;
    mInfoBlock = nullptr;
    mFileDataStart = nullptr;
    mDirs = nullptr;
    mFiles = nullptr;
    mStringTable = nullptr;
    _6C = false;
    mMountDir = mountDir;
    u32 expandedSize = 0;
    const auto direction = mountDir == MOUNT_DIRECTION_1
        ? JKRDvdRipper::ALLOC_DIRECTION_FORWARD : JKRDvdRipper::ALLOC_DIRECTION_BACKWARD;
    void* data = JKRDvdRipper::loadToMainRAM(entryNum, nullptr, EXPAND_SWITCH_UNKNOWN1,
        0, mHeap, direction, 0, reinterpret_cast<int*>(&_5C), &expandedSize);
    if (!data) { mMountMode = MOUNT_MODE_0; return false; }
    DCInvalidateRange(data, expandedSize);
    const bool registered = nativeRegisterRarc(data, expandedSize);
    const bool attached = registered && nativeAttachRarc(this, data);
    if (registered) nativeUnregisterRarc(data);
    JKRHeap::free(data, mHeap);
    if (!attached) mMountMode = MOUNT_MODE_0;
    return attached;
}
'@
$destructor='JKRMemArchive::~JKRMemArchive() {'
if([regex]::Matches($s,[regex]::Escape($destructor)).Count-ne 1){throw 'Memory archive destructor anchor'}
$s=$s.Replace($destructor,$destructor+' nativeDetachRarc(this);')
[IO.File]::WriteAllText("$n\src\generated\JKRMemArchive.cpp",'#include "../native_jkr_archive_port.hpp"'+[Environment]::NewLine+$s,[Text.UTF8Encoding]::new($false))
