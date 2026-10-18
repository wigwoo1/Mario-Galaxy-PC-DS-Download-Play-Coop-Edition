#include <Native/DrawSyncCompletion.hpp>
#include <revolution/gx/GXManage.h>
#include <future>
#include <cassert>
#include <cstdio>
static std::promise<void> entered,releaseHandler;
static auto releaseFuture=releaseHandler.get_future().share();
static void blocked(u16 token){assert(token==7);entered.set_value();releaseFuture.wait();}
static void selfRemove(u16 token){assert(token==9);assert(GXSetDrawSyncCallback(nullptr)==selfRemove);}
int main(){
 assert(GXSetDrawSyncCallback(blocked)==nullptr);
 auto worker=std::async(std::launch::async,[]{Native::dispatchDrawSyncCallback(7);});
 entered.get_future().wait();
 auto removal=std::async(std::launch::async,[]{return GXSetDrawSyncCallback(nullptr);});
 assert(removal.wait_for(std::chrono::milliseconds(20))==std::future_status::timeout);
 releaseHandler.set_value();worker.get();assert(removal.get()==blocked);
 Native::dispatchDrawSyncCallback(7);
 assert(GXSetDrawSyncCallback(selfRemove)==nullptr);Native::dispatchDrawSyncCallback(9);
 assert(GXSetDrawSyncCallback(nullptr)==nullptr);
 std::puts("PASS draw-sync callback unregister waits for in-flight handler and self-unregister is safe");
}
