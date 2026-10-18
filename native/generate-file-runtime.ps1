$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\FileUtil.cpp")
$s="#include <JSystem/JKernel/JKRMemArchive.hpp>`n#include <Native/HeapPlacement.hpp>`n"+$s
[IO.File]::WriteAllText("$n\src\generated\FileUtil.cpp",$s)
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\System\FileLoader.cpp")
if(!$s.Contains('mRequestedFileCount = nullptr;')){throw 'File count reset anchor'}
$s=$s.Replace('mRequestedFileCount = nullptr;','mRequestedFileCount = 0;')
[IO.File]::WriteAllText("$n\src\generated\FileLoader.cpp",$s)
foreach($unit in @('FileHolder','ArchiveHolder')){
 $s=[IO.File]::ReadAllText("$p\Petari\src\Game\System\$unit.cpp")
 $s="#include <JSystem/JKernel/JKRMemArchive.hpp>`n#include <Native/HeapPlacement.hpp>`n"+$s
 [IO.File]::WriteAllText("$n\src\generated\$unit.cpp",$s)
}
$fileHolderHeader=[IO.File]::ReadAllText("$p\Petari\include\Game\System\FileHolder.hpp")
$fileHolderHeader=$fileHolderHeader.Replace('#pragma once',"#pragma once`n#include <Native/ReadCompletion.hpp>")
$fileHolderHeader=$fileHolderHeader.Replace('bool mContextSet;',"bool mContextSet;`n    Native::ReadCompletion mNativeReadCompletion;")
if(!$fileHolderHeader.Contains('~FileHolderFileEntry();')){throw 'File-holder heap-delete declaration anchor'}
$fileHolderHeader=$fileHolderHeader.Replace('~FileHolderFileEntry();',"~FileHolderFileEntry();`n    static void operator delete(void*) noexcept;")
New-Item -ItemType Directory -Force "$n\include-runtime\Game\System" | Out-Null
[IO.File]::WriteAllText("$n\include-runtime\Game\System\FileHolder.hpp",$fileHolderHeader)
New-Item -ItemType Directory -Force "$n\include-runtime\JSystem\JKernel" | Out-Null
foreach($unit in @('JKRFileLoader','JKRArchive','JKRMemArchive','JKRAramArchive','JKRDvdArchive','JKRCompArchive')){
 $s=[IO.File]::ReadAllText("$p\Petari\libs\JSystem\include\JSystem\JKernel\$unit.hpp")
 $s=$s.Replace('unsigned long','u32')
 [IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\$unit.hpp",$s)
}
# Final-reference loader release must preserve virtual destruction and JKR ownership.
$loaderSource=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRFileLoader.cpp")
if(!$loaderSource.Contains('    delete this;')){throw 'File-loader unmount release anchor missing.'}
$loaderRelease='    JKRHeap* allocationHeap = JKRHeap::findFromRoot(this);'+[Environment]::NewLine+'    this->~JKRFileLoader();'+[Environment]::NewLine+'    JKRHeap::free(this, allocationHeap);'
$loaderSource='#include <JSystem/JKernel/JKRHeap.hpp>'+[Environment]::NewLine+$loaderSource.Replace('    delete this;',$loaderRelease)
[IO.File]::WriteAllText("$n\src\generated\JKRFileLoader.cpp",$loaderSource)
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\System\OSThreadWrapper.cpp")
$s="#include <JSystem/JKernel/JKRMemArchive.hpp>`n#include <Native/HeapPlacement.hpp>`n"+$s
[IO.File]::WriteAllText("$n\src\generated\OSThreadWrapper.cpp",$s)
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\System\FileRipper.cpp")
$anchor='(u32)(buf + 0x3f)'
if(!$s.Contains($anchor)){throw 'FileRipper buffer alignment anchor'}
$s=$s.Replace($anchor,'reinterpret_cast<std::uintptr_t>(buf + 0x3f)')
$s="#include <Native/HeapPlacement.hpp>`n#include <cstdint>`n"+$s
[IO.File]::WriteAllText("$n\src\generated\FileRipper.cpp",$s)
$f="$n\include-runtime\JSystem\JKernel\JKRArchive.hpp"
$h=[IO.File]::ReadAllText($f).Replace('#pragma once',"#pragma once`n#include <cstdint>")
$h=$h.Replace('s32 mEntryNum;','std::intptr_t mEntryNum;').Replace('check_mount_already(s32','check_mount_already(std::intptr_t')
[IO.File]::WriteAllText($f,$h)
$f="$n\include-runtime\JSystem\JKernel\JKRMemArchive.hpp"
$h=[IO.File]::ReadAllText($f).Replace('fixedInit(long)','fixedInit(std::intptr_t)')
[IO.File]::WriteAllText($f,$h)
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRMemArchive.cpp")
$s=$s.Replace('unsigned long','u32').Replace('fixedInit(long entryNum)','fixedInit(std::intptr_t entryNum)').Replace('reinterpret_cast< s32 >(a1)','reinterpret_cast<std::intptr_t>(a1)').Replace('reinterpret_cast< u32 >(mHeader)','reinterpret_cast<std::uintptr_t>(mHeader)')
$s=$s.Replace("case JKR_COMPRESSION_SZS:`r`n        u32 size", "case JKR_COMPRESSION_SZS: {`r`n        u32 size").Replace("return size;`r`n    default:","return size;`r`n    }`r`n    default:")
[IO.File]::WriteAllText("$n\src\generated\JKRMemArchive.cpp",$s)

$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRArchivePub.cpp")
$s=$s.Replace('unsigned long','u32').Replace('check_mount_already(long entryNum','check_mount_already(std::intptr_t entryNum').Replace('mount(long entryNum','mount(s32 entryNum')
$s="#include <JSystem/JKernel/JKRArchive.hpp>`n#include <Native/HeapPlacement.hpp>`n"+$s
[IO.File]::WriteAllText("$n\src\generated\JKRArchivePub.cpp",$s)

$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRThread.cpp")
$s="#include <Native/HeapPlacement.hpp>`n"+$s
[IO.File]::WriteAllText("$n\src\generated\JKRThread.cpp",$s)
$h=[IO.File]::ReadAllText("$p\Petari\libs\JSystem\include\JSystem\JKernel\JKRDecomp.hpp")
$h=$h.Replace('#pragma once',"#pragma once`n#include <cstdint>").Replace('void (*_14)(u32)','void (*_14)(std::uintptr_t)').Replace('void (*)(unsigned long)','void (*)(std::uintptr_t)').Replace('unsigned long','u32')
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRDecomp.hpp",$h)
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRDecomp.cpp")
$s=$s.Replace('void (*a5)(unsigned long)','void (*a5)(std::uintptr_t)').Replace('unsigned long','u32').Replace('reinterpret_cast< u32 >(&command)','reinterpret_cast<std::uintptr_t>(&command)')
$s="#include <JSystem/JKernel/JKRDecomp.hpp>`n#include <Native/HeapPlacement.hpp>`n"+$s
$deleteCommandAnchor='delete command;'
if([regex]::Matches($s,[regex]::Escape($deleteCommandAnchor)).Count -ne 1){throw 'JKRDecomp::orderSync command release anchor'}
$s=$s.Replace($deleteCommandAnchor,'command->~JKRDecompCommand(); JKRHeap::free(command, nullptr);')
$createQueueAnchor='if (sDecompObject == nullptr) {'
if([regex]::Matches($s,[regex]::Escape($createQueueAnchor)).Count -ne 1){throw 'JKRDecomp::create queue initialization anchor'}
$s=$s.Replace($createQueueAnchor,$createQueueAnchor+"`n        OSInitMessageQueue(&sMessageQueue, sMessageBuffer, 8);")
$runQueueAnchor='void\* JKRDecomp::run\(\) \{\r?\n\s*OSInitMessageQueue\(&sMessageQueue, sMessageBuffer, 8\);\r?\n'
if([regex]::Matches($s,$runQueueAnchor).Count -ne 1){throw 'JKRDecomp::run queue initialization anchor'}
$s=[regex]::Replace($s,$runQueueAnchor,'void* JKRDecomp::run() {',1)
$s=$s.Replace('u32 decompSize = ((s32)pDst','std::uintptr_t decompSize = (reinterpret_cast<std::uintptr_t>(pDst)').Replace('(u32)pDst != decompSize','reinterpret_cast<std::uintptr_t>(pDst) != decompSize')
$s=$s.Replace('*(u32*)(pSrc + 4)','READU32_BE(pSrc, 4)').Replace('*(u32*)pSrc','READU32_BE(pSrc, 0)')
[IO.File]::WriteAllText("$n\src\generated\JKRDecomp.cpp",$s)
$start=$s.IndexOf('#define READU32_BE');$end=$s.IndexOf('JKRDecompCommand::JKRDecompCommand()');if($start -lt 0 -or $end -le $start){throw 'Decoder kernel anchors'};[IO.File]::WriteAllText("$n\src\generated\JKRDecoderKernel.cpp","#include <JSystem/JKernel/JKRDecomp.hpp>`n"+$s.Substring($start,$end-$start))



$s=[IO.File]::ReadAllText("$n\src\generated\JKRMemArchive.cpp")
$s='#include "../native_jkr_archive_port.hpp"'+[Environment]::NewLine+$s
$signature='bool JKRMemArchive::open(void* pData, u32 a2, JKRMemBreakFlag breakFlag)'
$b=$s.IndexOf($signature);if($b -lt 0){throw 'memory archive open anchor'};$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
$body="{`n    if (!nativeAttachRarc(this, pData)) return false;`n    mHeap = JKRHeap::findFromRoot(pData);`n    _5C = 0;`n    return true;`n}"
$s=$s.Remove($o,$e-$o).Insert($o,$body)
$s=$s.Replace('_6C = breakFlag == JKR_MEM_BREAK_FLAG_1;','_6C = false;')
# Owned host tables are not heap allocations belonging to the source buffer.
$s=$s.Replace('JKRMemArchive::~JKRMemArchive() {','JKRMemArchive::~JKRMemArchive() { nativeDetachRarc(this);')
[IO.File]::WriteAllText("$n\src\generated\JKRMemArchive.cpp",$s)


$s=[IO.File]::ReadAllText("$n\src\generated\FileRipper.cpp")
$s='#include "../native_jkr_archive_port.hpp"'+[Environment]::NewLine+$s
$s=$s.Replace('return dest;','if (dest) nativeRegisterRarc(dest, compression ? decompressedSize : MR::getFileSize(fpath, false)); return dest;')
[IO.File]::WriteAllText("$n\src\generated\FileRipper.cpp",$s)
$s=[IO.File]::ReadAllText("$n\src\generated\FileHolder.cpp")
$waitPattern='(?s)void FileHolderFileEntry::waitReadDone\(\) \{.*?\r?\n\}'
$publishPattern='(?s)void FileHolderFileEntry::setContext\(void\* pContext, JKRHeap\* pHeap\) \{.*?\r?\n\}'
if([regex]::Matches($s,$waitPattern).Count-ne 1-or [regex]::Matches($s,$publishPattern).Count-ne 1){throw 'File completion method anchors'}
$s=[regex]::Replace($s,$waitPattern,'void FileHolderFileEntry::waitReadDone() { mNativeReadCompletion.wait(); }')
$s=[regex]::Replace($s,$publishPattern,'void FileHolderFileEntry::setContext(void* pContext, JKRHeap* pHeap) { mNativeReadCompletion.publish([&] { mContext = pContext; mHeap = pHeap; mState = FILE_HOLDER_STATE_DONE; }); }')
$s='#include "../native_jkr_archive_port.hpp"'+[Environment]::NewLine+$s
$s=$s.Replace('FileHolderFileEntry::~FileHolderFileEntry() {','FileHolderFileEntry::~FileHolderFileEntry() { nativeUnregisterRarc(mContext);')
$s=$s.Replace('delete (u8*)mContext;','JKRHeap::free(mContext, mHeap);')
$s+=[Environment]::NewLine+'void FileHolderFileEntry::operator delete(void* memory) noexcept { if (!memory) return; if (auto* heap = JKRHeap::findFromRoot(memory)) JKRHeap::free(memory, heap); else ::operator delete(memory); }'+[Environment]::NewLine
[IO.File]::WriteAllText("$n\src\generated\FileHolder.cpp",$s)
$s=[IO.File]::ReadAllText("$n\src\generated\ArchiveHolder.cpp")
$s='#include <stdexcept>'+[Environment]::NewLine+$s
$s=$s.Replace('archive->mountFixed(pData, JKR_MEM_BREAK_FLAG_0);','if (!archive->mountFixed(pData, JKR_MEM_BREAK_FLAG_0)) { archive->~JKRMemArchive(); JKRHeap::free(archive, pHeap); throw std::runtime_error("Native memory archive mount failed"); }')
$archiveNameDeleteNew='    JKRHeap::free(mArchiveName, mHeap);'
foreach($archiveNameDeleteOld in @('    delete mArchiveName;','    delete[] mArchiveName;')){if($s.Contains($archiveNameDeleteOld)){$s=$s.Replace($archiveNameDeleteOld,$archiveNameDeleteNew)}}
if(!$s.Contains($archiveNameDeleteNew)){throw 'ArchiveHolder name release anchor missing.'}
$archiveReleaseOld='    mArchive->unmount();' + [Environment]::NewLine + '    mArchive->~JKRMemArchive();' + [Environment]::NewLine + '    JKRHeap::free(mArchive, mHeap);' + [Environment]::NewLine + $archiveNameDeleteNew
$archiveReleaseNew='    mArchive->unmount();' + [Environment]::NewLine + $archiveNameDeleteNew
if($s.Contains($archiveReleaseOld)){$s=$s.Replace($archiveReleaseOld,$archiveReleaseNew)}elseif(!$s.Contains($archiveReleaseNew)){throw 'ArchiveHolder destructor release anchors missing.'}
$entryReleaseOld='            delete *i;'
$entryReleaseNew='            ArchiveHolderArchiveEntry* entry = *i;' + [Environment]::NewLine + '            JKRHeap* entryHeap = entry->mHeap;' + [Environment]::NewLine + '            entry->~ArchiveHolderArchiveEntry();' + [Environment]::NewLine + '            JKRHeap::free(entry, entryHeap);'
if($s.Contains($entryReleaseOld)){$s=$s.Replace($entryReleaseOld,$entryReleaseNew)}elseif(!$s.Contains($entryReleaseNew)){throw 'ArchiveHolder entry release anchor missing.'}
[IO.File]::WriteAllText("$n\src\generated\ArchiveHolder.cpp",$s)

# DVD completion owner must use a native pointer, not the Wii +0x3c overlay.
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRDvdFile.cpp")
$s=$s.Replace('mDvdFile = this;','mDvdFile = this; mFileInfo = {}; mFileInfo.cb.userData = this;')
$s=$s.Replace('JKRDvdFile* dvdFile = *(JKRDvdFile**)((u8*)fileInfo + 0x3c);','JKRDvdFile* dvdFile = static_cast<JKRDvdFile*>(fileInfo->cb.userData);')
# DVDFastOpen initializes the SDK structure, so install the owner after opening.
$s=$s.Replace('sDvdList.append(&mDvdLink);','mFileInfo.cb.userData = this; sDvdList.append(&mDvdLink);')
[IO.File]::WriteAllText("$n\src\generated\JKRDvdFile.cpp",$s)

$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRDvdRipper.cpp")
$s='#define ALIGN_NEXT(X,N) (((X)+(N)-1)&~(decltype(X))((N)-1))'+[Environment]::NewLine+'#define IS_NOT_ALIGNED(X,N) (((X)&((N)-1))!=0)'+[Environment]::NewLine+$s
$s=$s.Replace('!JKRDvdRipper::isErrorRetry)','!JKRDvdRipper::isErrorRetry())')
[IO.File]::WriteAllText("$n\src\generated\JKRDvdRipper.cpp",$s)

# Compilation bridge only: DVD archive disk-table overlay still needs host decoding.
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRDvdArchive.cpp")
$s='#include <cstdint>'+[Environment]::NewLine+'#include <cstddef>'+[Environment]::NewLine+'class JKRHeap; void* operator new(std::size_t, JKRHeap*, int);'+[Environment]::NewLine+'#define ALIGN_PREV(X,N) ((X)&~(decltype(X))((N)-1))'+[Environment]::NewLine+'#define ALIGN_NEXT(X,N) ALIGN_PREV((X)+(N)-1,N)'+[Environment]::NewLine+$s
$s=$s.Replace('reinterpret_cast< u32 >(headerBuffer)','reinterpret_cast< uintptr_t >(headerBuffer)')
[IO.File]::WriteAllText("$n\src\generated\JKRDvdArchive.cpp",$s)
$s=$s.Replace('reinterpret_cast< u32 >(buff)','reinterpret_cast< uintptr_t >(buff)')
[IO.File]::WriteAllText("$n\src\generated\JKRDvdArchive.cpp",$s)
$s='#include "../native_dvd_port.hpp"'+[Environment]::NewLine+'#include "../native_jkr_archive_port.hpp"'+[Environment]::NewLine+$s
$b=$s.IndexOf('bool JKRDvdArchive::open(s32 entryNum)');$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
$body=[IO.File]::ReadAllText("$n\src\dvd_archive_open.inc")
$s=$s.Remove($o,$e-$o).Insert($o,$body)
$s=$s.Replace('JKRFreeToHeap(mHeap, mInfoBlock);','nativeDetachRarcMetadata(this);')
[IO.File]::WriteAllText("$n\src\generated\JKRDvdArchive.cpp",$s)
$s='#include <new>'+[Environment]::NewLine+$s
$s=$s.Replace('mFile=new (JKRHeap::getSystemHeap(),0) JKRDvdFile(entryNum);','void* fileStorage=JKRHeap::alloc(sizeof(JKRDvdFile),32,JKRHeap::getSystemHeap()); mFile=fileStorage?new (fileStorage) JKRDvdFile(entryNum):nullptr;')
[IO.File]::WriteAllText("$n\src\generated\JKRDvdArchive.cpp",$s)
$heap=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRHeap.cpp")
$b=$heap.IndexOf('void JKRHeap::copyMemory(');$o=$heap.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($heap[$e] -eq '{'){$d++};if($heap[$e] -eq '}'){$d--};$e++}
[IO.File]::WriteAllText("$n\src\generated\HeapCopyKernel.cpp",'#include <JSystem/JKernel/JKRHeap.hpp>'+[Environment]::NewLine+$heap.Substring($b,$e-$b))
$s=[IO.File]::ReadAllText("$n\src\generated\JKRDvdRipper.cpp")
$s='#include <mutex>'+[Environment]::NewLine+$s
$b=$s.IndexOf('    BOOL interrupts = OSDisableInterrupts();');$end=$s.IndexOf('    OSRestoreInterrupts(interrupts);',$b)
if($b -lt 0 -or $end -lt $b){throw 'DVD decompressor initialization anchor'}
$s=$s.Remove($b,$end+'    OSRestoreInterrupts(interrupts);'.Length-$b).Insert($b,'    static std::once_flag initializeMutex; std::call_once(initializeMutex, [] { OSInitMutex(&decompMutex); });')
[IO.File]::WriteAllText("$n\src\generated\JKRDvdRipper.cpp",$s)
# Disc retry backoff is independent of the native presentation thread.
$s='#include <thread>'+[Environment]::NewLine+'#include <chrono>'+[Environment]::NewLine+$s
$s=$s.Replace('VIWaitForRetrace();','std::this_thread::sleep_for(std::chrono::microseconds(16667));')
[IO.File]::WriteAllText("$n\src\generated\JKRDvdRipper.cpp",$s)
$h=[IO.File]::ReadAllText("$p\Petari\libs\JSystem\include\JSystem\JKernel\JKRAramPiece.hpp")
$h='#include <cstdint>'+[Environment]::NewLine+$h
$h=$h.Replace('typedef void (*AsyncCallback)(u32);','typedef void (*AsyncCallback)(std::uintptr_t);').Replace('static void doneDMA(unsigned long);','static void doneDMA(std::uintptr_t);')
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRAramPiece.hpp",$h)
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRAramPiece.cpp")
$b=$s.IndexOf('void JKRAramPiece::doneDMA(');$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
$body=$s.Substring($b,$e-$b).Replace('doneDMA(u32 requestAddress)','doneDMA(std::uintptr_t requestAddress)')
[IO.File]::WriteAllText("$n\src\generated\AramCompletionKernel.cpp",'#include <JSystem/JKernel/JKRAramPiece.hpp>'+[Environment]::NewLine+'#include <JSystem/JKernel/JKRDecomp.hpp>'+[Environment]::NewLine+$body)
$h=[IO.File]::ReadAllText("$n\include-runtime\JSystem\JKernel\JKRCompArchive.hpp")
$h='#include <cstdint>'+[Environment]::NewLine+$h
$h=$h.Replace('JKRCompArchive(long,','JKRCompArchive(s32,').Replace('int field_0x64;','std::uintptr_t field_0x64;')
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRCompArchive.hpp",$h)
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRCompArchive.cpp")
$s='#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+'#define ALIGN_NEXT(X,N) (((X)+(N)-1)&~(decltype(X))((N)-1))'+[Environment]::NewLine+$s
[IO.File]::WriteAllText("$n\src\generated\JKRCompArchive.cpp",$s)
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRAramPiece.cpp")
$b=$s.IndexOf('JKRAMCommand::JKRAMCommand()');if($b -lt 0){throw 'ARAM command constructor anchor'}
[IO.File]::WriteAllText("$n\src\generated\AramCommandKernel.cpp",'#include <JSystem/JKernel/JKRAramPiece.hpp>'+[Environment]::NewLine+'#include <JSystem/JKernel/JKRHeap.hpp>'+[Environment]::NewLine+$s.Substring($b))
foreach($unit in @('JKRAramHeap','JKRAramBlock')){
 $s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\$unit.cpp")
 $s='#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+'#define ALIGN_PREV(X,N) ((X)&~(decltype(X))((N)-1))'+[Environment]::NewLine+'#define ALIGN_NEXT(X,N) ALIGN_PREV((X)+(N)-1,N)'+[Environment]::NewLine+$s
 [IO.File]::WriteAllText("$n\src\generated\$unit.cpp",$s)
}
$h=[IO.File]::ReadAllText("$p\Petari\libs\JSystem\include\JSystem\JKernel\JKRAramBlock.hpp")
$h=$h.Replace('virtual ~JKRAramBlock();','virtual ~JKRAramBlock(); static void operator delete(void*) noexcept;')
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRAramBlock.hpp",$h)
[IO.File]::AppendAllText("$n\src\generated\JKRAramBlock.cpp",[Environment]::NewLine+'void JKRAramBlock::operator delete(void* memory) noexcept { JKRHeap::free(memory, nullptr); }')
$s=[IO.File]::ReadAllText("$n\src\generated\JKRAramHeap.cpp")
$s=$s.Replace('u32 alignedSize = ALIGN_NEXT(size, 0x20);','if (!size || size > 0xffffffe0u) return nullptr; u32 alignedSize = ALIGN_NEXT(size, 0x20);')
[IO.File]::WriteAllText("$n\src\generated\JKRAramHeap.cpp",$s)
$s=[IO.File]::ReadAllText("$n\src\generated\JKRThread.cpp")
if(!$s.Contains('mMessageCount * 4')){throw 'Thread queue allocation anchor'}
$s=$s.Replace('mMessageCount * 4','mMessageCount * sizeof(OSMessage)')
[IO.File]::WriteAllText("$n\src\generated\JKRThread.cpp",$s)
$h=[IO.File]::ReadAllText("$p\Petari\libs\JSystem\include\JSystem\JKernel\JKRAramHeap.hpp")
$h=$h.Replace('virtual ~JKRAramHeap();','virtual ~JKRAramHeap(); static void operator delete(void*) noexcept;')
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRAramHeap.hpp",$h)
[IO.File]::AppendAllText("$n\src\generated\JKRAramHeap.cpp",[Environment]::NewLine+'void JKRAramHeap::operator delete(void* memory) noexcept { JKRHeap::free(memory, nullptr); }')
$h=[IO.File]::ReadAllText("$p\Petari\libs\JSystem\include\JSystem\JKernel\JKRDvdFile.hpp")
$h=$h.Replace('virtual ~JKRDvdFile();','virtual ~JKRDvdFile(); static void operator delete(void*) noexcept;')
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRDvdFile.hpp",$h)
[IO.File]::AppendAllText("$n\src\generated\JKRDvdFile.cpp",[Environment]::NewLine+'void JKRDvdFile::operator delete(void* memory) noexcept { JKRHeap::free(memory, nullptr); }')
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRAramPiece.cpp")
$b=$s.IndexOf('JKRAMCommand* JKRAramPiece::prepareCommand(');$e=$s.IndexOf('void JKRAramPiece::sendCommand(')
$part=$s.Substring($b,$e-$b)
$b=$s.IndexOf('JKRAMCommand* JKRAramPiece::orderAsync(');$e=$s.IndexOf('void JKRAramPiece::doneDMA(')
$part+=$s.Substring($b,$e-$b)
$part=$part.Replace('return TRUE;','return 1;').Replace('return FALSE;','return 0;')
[IO.File]::WriteAllText("$n\src\generated\AramQueueKernel.cpp",'#include <JSystem/JKernel/JKRAram.hpp>'+[Environment]::NewLine+'#include <JSystem/JKernel/JKRAramPiece.hpp>'+[Environment]::NewLine+'#include <JSystem/JUtility/JUTException.hpp>'+[Environment]::NewLine+'#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+'JSUList<JKRAMCommand> JKRAramPiece::sAramPieceCommandList;'+[Environment]::NewLine+$part)
$h=[IO.File]::ReadAllText("$n\include-runtime\JSystem\JKernel\JKRAramPiece.hpp")
$h=$h.Replace('~JKRAMCommand();','~JKRAMCommand(); static void operator delete(void*) noexcept;').Replace('static BOOL orderSync(int, unsigned long, unsigned long, unsigned long, JKRAramBlock*);','static BOOL orderSync(int, u32, u32, u32, JKRAramBlock*);')
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRAramPiece.hpp",$h)
[IO.File]::AppendAllText("$n\src\generated\AramCommandKernel.cpp",[Environment]::NewLine+'void JKRAMCommand::operator delete(void* memory) noexcept { JKRHeap::free(memory,nullptr); }')

$s=[IO.File]::ReadAllText("$n\src\generated\JKRCompArchive.cpp")
$s='#include "../native_dvd_port.hpp"'+[Environment]::NewLine+'#include "../native_jkr_archive_port.hpp"'+[Environment]::NewLine+'#include "../native_jkr_aram_port.hpp"'+[Environment]::NewLine+'#include "../native_rarc_tables.hpp"'+[Environment]::NewLine+'#include <new>'+[Environment]::NewLine+$s
$b=$s.IndexOf('bool JKRCompArchive::open(s32 entryNum)');$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
$s=$s.Remove($o,$e-$o).Insert($o,[IO.File]::ReadAllText("$n\src\comp_archive_open.inc"))
$s=$s.Replace('JKRFreeToHeap(mHeap, mInfoBlock);','nativeDetachRarcMetadata(this);')
[IO.File]::WriteAllText("$n\src\generated\JKRCompArchive.cpp",$s)
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRAramArchive.cpp")
$b=$s.IndexOf('u32 JKRAramArchive::fetchResource_subroutine(');$e=$s.IndexOf('u32 JKRAramArchive::getExpandedResSize(',$b)
$part=$s.Substring($b,$e-$b).Replace('reinterpret_cast< s32 >(&headerBuf[0])','reinterpret_cast< std::uintptr_t >(&headerBuf[0])')
[IO.File]::WriteAllText("$n\src\generated\AramArchiveFetchKernel.cpp",'#include <cstdint>'+[Environment]::NewLine+'#include <JSystem/JKernel/JKRAramArchive.hpp>'+[Environment]::NewLine+'#include <JSystem/JKernel/JKRAram.hpp>'+[Environment]::NewLine+'#include <JSystem/JKernel/JKRDecomp.hpp>'+[Environment]::NewLine+'#include <JSystem/JUtility/JUTException.hpp>'+[Environment]::NewLine+'#define ALIGN_PREV(X,N) ((X)&~(decltype(X))((N)-1))'+[Environment]::NewLine+'#define ALIGN_NEXT(X,N) ALIGN_PREV((X)+(N)-1,N)'+[Environment]::NewLine+$part)
$h=[IO.File]::ReadAllText("$n\include-runtime\JSystem\JKernel\JKRAramArchive.hpp")
$h=$h.Replace('JKRAramArchive(long,','JKRAramArchive(s32,').Replace('bool open(long);','bool open(s32);')
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRAramArchive.hpp",$h)
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRAramArchive.cpp")
$s=$s.Replace('JKRAramArchive::JKRAramArchive(long entryNum','JKRAramArchive::JKRAramArchive(s32 entryNum').Replace('reinterpret_cast< s32 >','reinterpret_cast< std::uintptr_t >')
$s='#include "../native_dvd_port.hpp"'+[Environment]::NewLine+'#include "../native_jkr_archive_port.hpp"'+[Environment]::NewLine+'#include "../native_jkr_aram_port.hpp"'+[Environment]::NewLine+'#include "../native_rarc_tables.hpp"'+[Environment]::NewLine+'#include <JSystem/JKernel/JKRDecomp.hpp>'+[Environment]::NewLine+'#include <new>'+[Environment]::NewLine+'#define ALIGN_PREV(X,N) ((X)&~(decltype(X))((N)-1))'+[Environment]::NewLine+'#define ALIGN_NEXT(X,N) ALIGN_PREV((X)+(N)-1,N)'+[Environment]::NewLine+$s
$b=$s.IndexOf('bool JKRAramArchive::open(s32 entryNum)');$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
$s=$s.Remove($o,$e-$o).Insert($o,[IO.File]::ReadAllText("$n\src\aram_archive_open.inc"))
$s=$s.Replace('JKRFreeToHeap(mHeap, mInfoBlock);','nativeDetachRarcMetadata(this);')
[IO.File]::WriteAllText("$n\src\generated\JKRAramArchive.cpp",$s)


