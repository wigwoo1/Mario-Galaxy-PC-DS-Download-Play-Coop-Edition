#include <Native/DrawCompletion.hpp>
#include <revolution/gx/GXManage.h>
#include <mutex>
namespace {std::mutex callbackMutex;GXDrawDoneCallback drawDoneCallback=nullptr;}
extern "C" GXDrawDoneCallback GXSetDrawDoneCallback(GXDrawDoneCallback callback){
 std::lock_guard<std::mutex> lock(callbackMutex);auto old=drawDoneCallback;drawDoneCallback=callback;return old;
}
void Native::dispatchDrawDoneCallback(){
 GXDrawDoneCallback callback;
 {std::lock_guard<std::mutex> lock(callbackMutex);callback=drawDoneCallback;}
 if(callback)callback();
}
