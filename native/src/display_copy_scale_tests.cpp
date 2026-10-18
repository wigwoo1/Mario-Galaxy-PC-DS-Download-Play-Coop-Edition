#include <initializer_list>
#include <revolution/gx.h>
#include <cassert>
#include <cstdio>
int main(){
 for(u16 h: {u16(240),u16(448),u16(480),u16(528)}){
 assert(GXGetNumXfbLines(h,1.0f)==h);
 assert(GXGetYScaleFactor(h,h)==1.0f);
 }
 assert(GXGetNumXfbLines(384,2.0f)==767);
 assert(GXGetNumXfbLines(600,2.0f)==1024);
 for(u16 target: {u16(480),u16(528),u16(576)}){
 auto scale=GXGetYScaleFactor(448,target);
 auto lines=GXGetNumXfbLines(448,scale);
 assert(scale>=1.0f && scale<1.3f && lines<=target && target-lines<=2);
 }
 std::puts("PASS original display copy identity, doubling, line clamp and target-height scale selection");
}

