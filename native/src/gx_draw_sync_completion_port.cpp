#include <Native/DrawSyncCompletion.hpp>
#include <revolution/gx/GXManage.h>
#include <mutex>
namespace {
 std::recursive_mutex callbackMutex;
 GXDrawSyncCallback currentCallback=nullptr;
}
extern "C" GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback callback){
 std::lock_guard<std::recursive_mutex> lock(callbackMutex);
 auto previous=currentCallback;currentCallback=callback;return previous;
}
void Native::dispatchDrawSyncCallback(std::uint16_t token){
 // Hold through invocation: unregistering on another thread waits for callback completion.
 // Recursive locking allows a handler to replace/unregister itself.
 std::lock_guard<std::recursive_mutex> lock(callbackMutex);
 if(currentCallback)currentCallback(token);
}
