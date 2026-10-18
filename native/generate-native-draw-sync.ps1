$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\DrawSyncManager.cpp" -Raw
$prefix=@"
#include <memory>
namespace {
enum class NativeDrawMessageKind { BreakPoint, Token, Shutdown };
struct NativeDrawMessage { NativeDrawMessageKind kind; void* pointer; };
void nativeSendDrawMessage(OSMessageQueue* queue, NativeDrawMessageKind kind, void* pointer=nullptr) {
    auto message=std::make_unique<NativeDrawMessage>(NativeDrawMessage{kind,pointer});
    if(OSSendMessage(queue,message.get(),OS_MESSAGE_BLOCK)) message.release();
}
}
"@
$s=$s.Replace('DrawSyncManager* DrawSyncManager::sInstance;', $prefix+[Environment]::NewLine+'DrawSyncManager* DrawSyncManager::sInstance;')
$s=$s.Replace('if (reinterpret_cast< u32 >(message) >= 0x80000000) {','std::unique_ptr<NativeDrawMessage> payload(static_cast<NativeDrawMessage*>(message));'+[Environment]::NewLine+'        if (payload->kind == NativeDrawMessageKind::BreakPoint) {')
$s=$s.Replace('pManager->mFifo->push(message);','pManager->mFifo->push(payload->pointer);')
$s=$s.Replace('GXEnableBreakPt(message);','GXEnableBreakPt(payload->pointer);')
$s=$s.Replace('} else if (reinterpret_cast< u32 >(message) < 0x10000) {','} else if (payload->kind == NativeDrawMessageKind::Token) {')
$s=$s.Replace('OSSendMessage(&mQueue, reinterpret_cast< OSMessage >(0x10000), OS_MESSAGE_BLOCK);','nativeSendDrawMessage(&mQueue, NativeDrawMessageKind::Shutdown);')
$s=$s.Replace('OSSendMessage(&mQueue, reinterpret_cast< OSMessage >(token), OS_MESSAGE_BLOCK);','nativeSendDrawMessage(&mQueue, NativeDrawMessageKind::Token);')
$s=$s.Replace('OSSendMessage(&mQueue, pWrite, OS_MESSAGE_BLOCK);','nativeSendDrawMessage(&mQueue, NativeDrawMessageKind::BreakPoint, pWrite);')
$s=$s.Replace('OSJoinThread(&mThread, nullptr);','OSJoinThread(&mThread, nullptr);'+[Environment]::NewLine+'    GXDisableBreakPt(); delete[] mFifo->mArray; delete mFifo; delete[] mMessageBuffer; delete[] mStack;')
if($s.Contains('reinterpret_cast< u32 >')){throw 'Remaining pointer classification'}
Set-Content "$n\src\generated\DrawSyncManager.cpp" $s
$a=$s.IndexOf('void* Fifo::pop()');$b=$s.IndexOf('void DrawSyncManager::end()', $a)
$c=$s.IndexOf('void* DrawSyncManager::threadFunc(');$d=$s.IndexOf('DrawSyncManager::DrawSyncManager(', $c)
$prefix='#include <Game/System/DrawSyncManager.hpp>'+[Environment]::NewLine+'#include <memory>'+[Environment]::NewLine+'namespace { enum class NativeDrawMessageKind { BreakPoint, Token, Shutdown }; struct NativeDrawMessage { NativeDrawMessageKind kind; void* pointer; }; }'+[Environment]::NewLine
Set-Content "$n\src\generated\DrawSyncWorkerFixture.cpp" ($prefix+$s.Substring($a,$b-$a)+$s.Substring($c,$d-$c))

$prefixEnd=$s.IndexOf('DrawSyncManager* DrawSyncManager::sInstance;')
$prefix=$s.Substring(0,$prefixEnd)+'DrawSyncManager* DrawSyncManager::sInstance;'+[Environment]::NewLine
$start=$s.IndexOf('void* Fifo::pop()');$stop=$s.IndexOf('void DrawSyncManager::end()', $start)
$thread=$s.IndexOf('void* DrawSyncManager::threadFunc(');$reset=$s.IndexOf('void DrawSyncManager::reset(', $thread)
$callback=$s.IndexOf('void DrawSyncManager::drawSyncCallbackSub(');$push=$s.IndexOf('void DrawSyncManager::pushBreakPoint()', $callback)
$dispatch='void DrawSyncManager::drawSyncCallback(u16 token){if(sInstance)sInstance->drawSyncCallbackSub(token);}'+[Environment]::NewLine
Set-Content "$n\src\generated\DrawSyncLiveFixture.cpp" ($prefix+$s.Substring($start,$stop-$start)+$dispatch+$s.Substring($thread,$reset-$thread)+$s.Substring($callback))

