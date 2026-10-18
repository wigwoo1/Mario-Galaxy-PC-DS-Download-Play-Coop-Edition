#include <revolution/mtx.h>
#include <cmath>

extern "C" u32 PSMTXInvXpose(const Mtx src, Mtx dst) {
    const f32 a=src[0][0], b=src[0][1], c=src[0][2];
    const f32 d=src[1][0], e=src[1][1], f=src[1][2];
    const f32 g=src[2][0], h=src[2][1], i=src[2][2];
    const f32 cofactors[3][3]={
        {e*i-f*h, f*g-d*i, d*h-e*g},
        {c*h-b*i, a*i-c*g, b*g-a*h},
        {b*f-c*e, c*d-a*f, a*e-b*d}
    };
    const f32 determinant=a*cofactors[0][0]+b*cofactors[0][1]+c*cofactors[0][2];
    if(determinant==0.0f || !std::isfinite(determinant))return 0;
    const f32 inverseDeterminant=1.0f/determinant;
    Mtx result;
    for(int row=0;row<3;++row){
        for(int column=0;column<3;++column)
            result[row][column]=cofactors[row][column]*inverseDeterminant;
        result[row][3]=0.0f;
    }
    for(int row=0;row<3;++row)
        for(int column=0;column<4;++column)
            dst[row][column]=result[row][column];
    return 1;
}

extern "C" void C_MTXLookAt(Mtx matrix,const Point3d* camera,const Vec* worldUp,const Point3d* target) {
    Vec look{camera->x-target->x,camera->y-target->y,camera->z-target->z};
    const f32 lookLength=std::sqrt(look.x*look.x+look.y*look.y+look.z*look.z);
    look.x/=lookLength;look.y/=lookLength;look.z/=lookLength;
    Vec right{
        worldUp->y*look.z-worldUp->z*look.y,
        worldUp->z*look.x-worldUp->x*look.z,
        worldUp->x*look.y-worldUp->y*look.x
    };
    const f32 rightLength=std::sqrt(right.x*right.x+right.y*right.y+right.z*right.z);
    right.x/=rightLength;right.y/=rightLength;right.z/=rightLength;
    const Vec up{
        look.y*right.z-look.z*right.y,
        look.z*right.x-look.x*right.z,
        look.x*right.y-look.y*right.x
    };
    matrix[0][0]=right.x;matrix[0][1]=right.y;matrix[0][2]=right.z;
    matrix[0][3]=-(camera->x*right.x+camera->y*right.y+camera->z*right.z);
    matrix[1][0]=up.x;matrix[1][1]=up.y;matrix[1][2]=up.z;
    matrix[1][3]=-(camera->x*up.x+camera->y*up.y+camera->z*up.z);
    matrix[2][0]=look.x;matrix[2][1]=look.y;matrix[2][2]=look.z;
    matrix[2][3]=-(camera->x*look.x+camera->y*look.y+camera->z*look.z);
}
