#include <JSystem/JMath/JMath.hpp>
#include <cassert>
#include <cstdio>
int main(){
    Quaternion a{0,0,0,1},b{0,0,0,-1},q;JMAQuatLerp(&a,&b,.5f,&q);assert(q.w==1&&q.x==0);
    b={2,0,0,1};JMAQuatLerp(&a,&b,.25f,&a);assert(a.x==.5f&&a.w==1);
    Vec x{1,2,3},y{5,6,7};JMAVECLerp(&x,&y,&x,.5f);assert(x.x==3&&x.y==4&&x.z==5);
    JMAVECScaleAdd(&x,&y,&y,-2);assert(y.x==-1&&y.y==-2&&y.z==-3);
    Mtx m={{1,2,3,7},{4,5,6,-5},{7,8,9,9}};JMAMTXApplyScale(m,m,2,-3,.5f);
    assert(m[0][0]==2&&m[1][1]==-15&&m[2][2]==4.5f&&m[0][3]==7&&m[1][3]==-5&&m[2][3]==9);
    JMAEulerToQuat(0,0,0,&q);assert(q.x==0&&q.y==0&&q.z==0&&q.w==1);
    std::puts("PASS native JMath quaternion hemisphere, vector aliasing, matrix scaling and original Euler conversion");
}
