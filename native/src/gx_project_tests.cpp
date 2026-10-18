#include <revolution/mtx.h>
#include <revolution/gx/GXTransform.h>
#include <cmath>
#include <cstdio>
int main(){Mtx m={{1,0,0,0},{0,1,0,0},{0,0,1,0}};float vp[]={10,20,640,480,0,1},p[]={0,1,0,1,0,-1,-1},x,y,z;
 GXProject(0,0,-10,m,p,vp,&x,&y,&z);if(std::abs(x-330)>0.001f||std::abs(y-260)>0.001f||std::abs(z-1.9f)>0.001f)return 1;
 p[0]=1;p[5]=0;p[6]=0;GXProject(1,1,0,m,p,vp,&x,&y,&z);if(std::abs(x-650)>0.001f||std::abs(y-20)>0.001f||std::abs(z-1)>0.001f)return 2;
 m[0][3]=-1;m[1][3]=-1;GXProject(1,1,0,m,p,vp,&x,&y,&z);if(std::abs(x-330)>0.001f||std::abs(y-260)>0.001f)return 3;
 std::puts("PASS original GXProject perspective, orthographic and model translation");}

