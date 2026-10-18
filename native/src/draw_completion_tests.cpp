#include <Native/DrawCompletion.hpp>
#include <revolution/gx/GXManage.h>
#include <cassert>
#include <cstdio>
int count=0;
void second(){count+=10;}
void first(){++count;assert(GXSetDrawDoneCallback(second)==first);}
int main(){
 assert(GXSetDrawDoneCallback(first)==nullptr);assert(count==0);
 Native::dispatchDrawDoneCallback();assert(count==1);
 Native::dispatchDrawDoneCallback();assert(count==11);
 assert(GXSetDrawDoneCallback(nullptr)==second);Native::dispatchDrawDoneCallback();assert(count==11);
 std::puts("PASS draw-done registration: previous callback, no premature completion, reentrant replacement and removal");
}
