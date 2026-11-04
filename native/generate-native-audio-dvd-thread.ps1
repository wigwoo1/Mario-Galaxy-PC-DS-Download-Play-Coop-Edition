$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\JAudio2\JASDvdThread.cpp'))
$anchor='    static JASTaskThread* sThread;'
$owner=@'
    static JASTaskThread* sThread;
    // Base destruction runs in reverse order: retain the published reference
    // until JASTaskThread/JKRThread have drained/cancelled and joined the worker.
    // Creation/destruction are serialized; the factory permits one live worker.
    class NativeDvdThreadReference {
    protected:
        ~NativeDvdThreadReference() { sThread = nullptr; }
    };
    class NativeDvdTaskThread final : private NativeDvdThreadReference, public JASTaskThread {
    public:
        using JASTaskThread::JASTaskThread;
    };
    static_assert(sizeof(NativeDvdTaskThread) == sizeof(JASTaskThread), "DVD worker native ABI changed");
'@
if([regex]::Matches($source,[regex]::Escape($anchor)).Count-ne 1){throw 'Original DVD singleton anchor changed'}
$source=$source.Replace($anchor,$owner)
$allocation='new (JASDram, 0) JASTaskThread(priority, msgCount, stackSize)'
if([regex]::Matches($source,[regex]::Escape($allocation)).Count-ne 1){throw 'Original DVD worker allocation anchor changed'}
$source=$source.Replace($allocation,'new (JASDram, 0) NativeDvdTaskThread(priority, msgCount, stackSize)')
[IO.File]::WriteAllText((Join-Path $n 'src\generated\AudioDvdThreadKernel.cpp'),"#include <Native/HeapPlacement.hpp>`n"+$source,[Text.UTF8Encoding]::new($false))
