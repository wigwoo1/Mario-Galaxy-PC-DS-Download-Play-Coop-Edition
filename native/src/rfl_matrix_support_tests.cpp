#include <revolution/mtx.h>
#include <cassert>
#include <cmath>

extern "C" u32 PSMTXInvXpose(const Mtx src,Mtx dst);
extern "C" void C_MTXLookAt(Mtx matrix,const Point3d* camera,const Vec* up,const Point3d* target);

static void near(f32 actual,f32 expected){assert(std::fabs(actual-expected)<1.0e-5f);}

int main(){
    Mtx scale{{2,0,0,9},{0,4,0,8},{0,0,5,7}},normal{};
    assert(PSMTXInvXpose(scale,normal)==1);
    near(normal[0][0],0.5f);near(normal[1][1],0.25f);near(normal[2][2],0.2f);
    near(normal[0][3],0);near(normal[1][3],0);near(normal[2][3],0);
    near(normal[0][1],0);near(normal[1][0],0);near(normal[2][1],0);

    Mtx shear{{1,2,0,3},{0,1,0,4},{0,0,2,5}},inverseTranspose{};
    assert(PSMTXInvXpose(shear,inverseTranspose)==1);
    near(inverseTranspose[0][0],1);near(inverseTranspose[0][1],0);
    near(inverseTranspose[1][0],-2);near(inverseTranspose[1][1],1);
    near(inverseTranspose[2][2],0.5f);

    Mtx singular{{1,2,3,0},{2,4,6,0},{0,0,0,0}},untouched{{7,7,7,7},{7,7,7,7},{7,7,7,7}};
    assert(PSMTXInvXpose(singular,untouched)==0);
    near(untouched[1][2],7);

    const Point3d camera{0,0,5},target{0,0,0};const Vec up{0,1,0};Mtx view{};
    C_MTXLookAt(view,&camera,&up,&target);
    near(view[0][0],1);near(view[1][1],1);near(view[2][2],1);
    near(view[0][3],0);near(view[1][3],0);near(view[2][3],-5);
    return 0;
}
