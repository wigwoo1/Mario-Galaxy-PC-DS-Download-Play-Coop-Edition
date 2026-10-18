#include <JSystem/JGeometry/TMatrix.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
 TVec3f v(3,4,0);assert(v.normalize()==5 && std::fabs(v.x-.6f)<1e-6f && std::fabs(v.y-.8f)<1e-6f);
 TVec3f w;assert(w.normalize(TVec3f(0,0,-2))==2 && w.z==-1);assert(w.normalize(w)==1 && w.z==-1);
 TPos3f scaled;scaled.set(0,-3,0,99,2,0,0,88,0,0,-4,77);TVec3f scales;scaled.getScale(scales);assert(scales.x==2 && scales.y==3 && scales.z==4);
 TPos3f src,dst;src.set(1,2,3,4,5,6,7,8,9,10,11,12);dst.setPositionFromLookAt(src);
 const float expected[3][4]={{-1,5,-9,-152},{-2,6,-10,-176},{-3,7,-11,-200}};
 for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(dst.get(r,c)==expected[r][c]);
 src.setPositionFromLookAt(src);for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(src.get(r,c)==expected[r][c]);
 src.identity();src.setTrans(-1,-2,-3);dst.setPositionFromLookAt(src);assert(dst.get(0,3)==1 && dst.get(1,3)==2 && dst.get(2,3)==3);
 std::puts("PASS audio listener transform and vector normalization");
}
