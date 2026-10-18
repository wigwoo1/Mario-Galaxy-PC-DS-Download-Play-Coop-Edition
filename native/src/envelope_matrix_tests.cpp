#include "Native/EnvelopeMatrix.hpp"
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
    Mtx world[2]={{{2,1,0,7},{0,3,1,-5},{1,0,-4,9}},{{0,-1,0,3},{1,0,0,4},{0,0,2,-6}}};
    Mtx inverse[2]={{{1,0,0,-2},{0,1,0,3},{0,0,1,-1}},{{1,2,0,5},{0,1,0,6},{0,0,1,7}}};
    u16 indices[2]={1,0};f32 weights[2]={.25f,.75f};u8 flags[2]={1,1};Mtx result;
    assert(Native::envelopeMatrix(world,inverse,indices,weights,flags,2,result)==1);
    Vec point{3,-2,6},expected{0,0,0},actual;
    for(unsigned j=0;j<2;++j){Vec bound,transformed;PSMTXMultVec(inverse[indices[j]],&point,&bound);PSMTXMultVec(world[indices[j]],&bound,&transformed);expected.x+=weights[j]*transformed.x;expected.y+=weights[j]*transformed.y;expected.z+=weights[j]*transformed.z;}
    PSMTXMultVec(result,&point,&actual);
    assert(std::fabs(actual.x-expected.x)<.00001f&&std::fabs(actual.y-expected.y)<.00001f&&std::fabs(actual.z-expected.z)<.00001f);
    flags[0]=0;assert(Native::envelopeMatrix(world,inverse,indices,weights,flags,2,result)==0);
    weights[0]=.5f;weights[1]=1.5f;Native::envelopeMatrix(world,inverse,indices,weights,flags,2,result);
    PSMTXMultVec(result,&point,&actual);assert(std::fabs(actual.x-2*expected.x)<.00001f);
    assert(Native::envelopeMatrix(world,inverse,indices,weights,flags,0,result)==1);
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(result[r][c]==0);
    std::puts("PASS weighted joint/inverse-bind composition, influence order, unnormalized weights and scale flags");
}
