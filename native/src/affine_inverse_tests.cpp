#include <revolution/mtx.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdio>
int main(){Mtx matrix={{0,-3,1,100},{2,0,.5f,-80},{0,0,-4,27}},inverse;
 assert(PSMTXInverse(matrix,inverse)==1);for(int row=0;row<3;++row)for(int col=0;col<4;++col){float value=col==3?inverse[row][3]:0;for(int k=0;k<3;++k)value+=inverse[row][k]*matrix[k][col];assert(std::abs(value-(row==col?1.0f:0.0f))<.0001f);}
 Mtx inplace;std::memcpy(inplace,matrix,sizeof(Mtx));assert(PSMTXInverse(inplace,inplace)==1);for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(inplace[r][c]==inverse[r][c]);
 Vec point{11,-3,9},world,back;PSMTXMultVec(matrix,&point,&world);PSMTXMultVec(inverse,&world,&back);assert(std::abs(back.x-point.x)<.0001f && std::abs(back.y-point.y)<.0001f && std::abs(back.z-point.z)<.0001f);
 Mtx singular={};Mtx before;std::memcpy(before,inverse,sizeof(Mtx));assert(PSMTXInverse(singular,inverse)==0 && std::memcmp(before,inverse,sizeof(Mtx))==0);
 std::puts("PASS native affine inverse, aliasing, transformed point roundtrip and singular rejection");}
