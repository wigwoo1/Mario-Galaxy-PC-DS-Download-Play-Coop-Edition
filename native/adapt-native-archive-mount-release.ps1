$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$path=Join-Path $n 'src\generated\JKRArchivePub.cpp'
$s=[IO.File]::ReadAllText($path)
$old='        delete archive;'
$new='        JKRHeap* allocationHeap = JKRHeap::findFromRoot(archive);'+[Environment]::NewLine+'        archive->~JKRArchive();'+[Environment]::NewLine+'        JKRHeap::free(archive, allocationHeap);'
$count=[regex]::Matches($s,[regex]::Escape($old)).Count
if($count-eq 1){$s=$s.Replace($old,$new)}
elseif($count-ne 0 -or [regex]::Matches($s,[regex]::Escape($new)).Count-ne 1){throw 'Failed archive mount release anchor changed'}
[IO.File]::WriteAllText($path,$s,[Text.UTF8Encoding]::new($false))
