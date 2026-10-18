#include <JSystem/JGeometry.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
 TProj3f a,b,c;
 for(int r=0;r<4;++r)for(int col=0;col<4;++col){a.mMtx[r][col]=float(r*4+col-7);b.mMtx[r][col]=float(3*r-col+2);}
 c.concat(a,b);auto left=a,right=b;left.concat(left,b);right.concat(a,right);
 for(int r=0;r<4;++r)for(int col=0;col<4;++col){double expected=0;for(int k=0;k<4;++k)expected+=double(a.mMtx[r][k])*b.mMtx[k][col];assert(c.mMtx[r][col]==expected&&left.mMtx[r][col]==expected&&right.mMtx[r][col]==expected);}
 auto square=a;square.concat(square,square);
 for(int r=0;r<4;++r)for(int col=0;col<4;++col){float expected=0;for(int k=0;k<4;++k)expected+=a.mMtx[r][k]*a.mMtx[k][col];assert(square.mMtx[r][col]==expected);}
 TProj3f projection,translation,shifted;projection.makePerspective(45,16.f/9,100,800000);translation.makeTrans(.125f,-.25f);shifted.concat(translation,projection);
 for(int col=0;col<4;++col){assert(std::fabs(shifted.mMtx[0][col]-(projection.mMtx[0][col]+.125f*projection.mMtx[3][col]))<1e-6f);assert(std::fabs(shifted.mMtx[1][col]-(projection.mMtx[1][col]-.25f*projection.mMtx[3][col]))<1e-6f);}
 TProj3f vectorTranslation;vectorTranslation.makeTrans(TVec2f(.125f,-.25f));for(int r=0;r<4;++r)for(int col=0;col<4;++col)assert(vectorTranslation.mMtx[r][col]==translation.mMtx[r][col]);
 std::puts("PASS projection concatenation: independent multiplication, left/right/both aliasing, original camera shake order");
}
