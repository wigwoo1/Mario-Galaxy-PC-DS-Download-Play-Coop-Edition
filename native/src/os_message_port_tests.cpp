#include <revolution/types.h>
#include <revolution/os/OSMessage.h>
#ifdef nullptr
#undef nullptr
#endif
#include <cassert>
#include <cstdint>
#include <future>
#include <thread>
#include <cstdio>
OSMessage value(uintptr_t n){return reinterpret_cast<OSMessage>(n);}
int main(){OSMessageQueue queue;OSMessage storage[3];OSInitMessageQueue(&queue,storage,3);OSMessage out=value(99);assert(!OSReceiveMessage(&queue,&out,0) && out==value(99));assert(OSSendMessage(&queue,value(1),0));assert(OSSendMessage(&queue,value(2),0));assert(OSJamMessage(&queue,value(3),0));assert(!OSSendMessage(&queue,value(4),0));assert(!OSJamMessage(&queue,value(4),0));for(auto n:{3,1,2}){assert(OSReceiveMessage(&queue,&out,0) && out==value(n));}assert(OSSendMessage(&queue,nullptr,0));assert(OSReceiveMessage(&queue,&out,0) && out==nullptr);
 constexpr uintptr_t jobs=20000;std::thread producer([&]{for(uintptr_t n=1;n<=jobs;++n)assert(OSSendMessage(&queue,value(n),OS_MESSAGE_BLOCK));});std::thread consumer([&]{for(uintptr_t n=1;n<=jobs;++n){OSMessage received;assert(OSReceiveMessage(&queue,&received,OS_MESSAGE_BLOCK));assert(received==value(n));}});producer.join();consumer.join();assert(queue.usedCount==0);
 assert(OSSendMessage(&queue,value(7),0));assert(OSReceiveMessage(&queue,nullptr,0));OSInitMessageQueue(&queue,storage,1);assert(OSSendMessage(&queue,value(8),0));assert(!OSSendMessage(&queue,value(9),0));assert(OSReceiveMessage(&queue,&out,0) && out==value(8));std::puts("PASS native OS queue FIFO, jam, wraparound, null and 20000 threaded jobs");}

