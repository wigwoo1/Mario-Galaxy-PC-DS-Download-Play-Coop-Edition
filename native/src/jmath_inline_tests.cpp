#include <JSystem/JMath/JMath.hpp>
#include <cassert>
#include <initializer_list>
#include <cstdio>
int main(){
    assert(JMAFastSqrt(9)==3 && JMAFastSqrt(0)==0 && JMAFastSqrt(-2)==-2);
    for(unsigned tick=0;tick<=100;++tick){float time=tick/10.0f;
        assert(std::fabs(JMAHermiteInterpolation(time,0,2,3,10,32,3)-(2+3*time))<0.0001f);
    }
    assert(JMAHermiteInterpolation(2,2,-4,3,6,8,-2)==-4);
    assert(JMAHermiteInterpolation(6,2,-4,3,6,8,-2)==8);
    assert(JMAHermiteInterpolation(4,2,-4,0,6,8,0)==2);
    struct CopyCase {unsigned count;void (*copy)(void*,const void*);};
    for(auto entry:{CopyCase{3,JMath::gekko_ps_copy3},CopyCase{6,JMath::gekko_ps_copy6},CopyCase{12,JMath::gekko_ps_copy12},CopyCase{16,JMath::gekko_ps_copy16}}){
        u32 source[18],destination[18];for(unsigned i=0;i<18;++i){source[i]=0x3f000000u+i*12345u;destination[i]=0xdeadbeefu;}
        entry.copy(destination+1,source+1);
        assert(destination[0]==0xdeadbeefu && destination[entry.count+1]==0xdeadbeefu);
        for(unsigned i=1;i<=entry.count;++i)assert(destination[i]==source[i]);
        entry.copy(destination+1,destination+1);
        for(unsigned i=1;i<=entry.count;++i)assert(destination[i]==source[i]);
    }
    std::puts("PASS native paired copies: exact 3/6/12/16-word payloads, guards and self-copy");
    std::puts("PASS native inline sqrt and Hermite endpoints, midpoint and 101 linear-track samples");
}


