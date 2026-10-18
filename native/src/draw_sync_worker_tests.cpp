#include "generated/DrawSyncWorkerFixture.cpp"
#include <cassert>
#include <vector>
#include <cstdio>
static std::vector<OSMessage> messages;
static size_t cursor;
static std::vector<void*> enabled;
static unsigned disabled;
extern "C" BOOL OSReceiveMessage(OSMessageQueue*,OSMessage* out,s32){assert(cursor<messages.size());*out=messages[cursor++];return 1;}
extern "C" void GXEnableBreakPt(void* pointer){enabled.push_back(pointer);}
extern "C" void GXDisableBreakPt(){++disabled;}
DrawSyncManager::DrawSyncManager(u32,s32):mFifo(nullptr),mFlags(0),mAborted(false){}
DrawSyncManager::~DrawSyncManager(){}
int main(){
 // Test-only constructor-free worker fixture; no GPU or device execution.
 alignas(DrawSyncManager) unsigned char storage[sizeof(DrawSyncManager)]{};
 auto* manager=reinterpret_cast<DrawSyncManager*>(storage);
 Fifo fifo(4);manager->mFifo=&fifo;
 auto* high=reinterpret_cast<void*>(uintptr_t(0x123456789abcdef0ull));
 auto* low=reinterpret_cast<void*>(uintptr_t(0x1234));
 messages.push_back(new NativeDrawMessage{NativeDrawMessageKind::BreakPoint,high});
 messages.push_back(new NativeDrawMessage{NativeDrawMessageKind::BreakPoint,low});
 messages.push_back(new NativeDrawMessage{NativeDrawMessageKind::Token,nullptr});
 messages.push_back(new NativeDrawMessage{NativeDrawMessageKind::Token,nullptr});
 messages.push_back(new NativeDrawMessage{NativeDrawMessageKind::Shutdown,nullptr});
 assert(DrawSyncManager::threadFunc(manager)==nullptr);
 assert(cursor==5&&enabled.size()==1&&enabled[0]==low&&disabled==1&&fifo.getCount()==0);
 delete[] fifo.mArray;
 std::puts("PASS original adapted worker explicit message types and 64-bit/low-address pointer preservation (fixture transport)");
}

