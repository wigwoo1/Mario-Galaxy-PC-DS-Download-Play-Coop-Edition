#include <revolution/mtx.h>
#include <JSystem/JGeometry/TVec.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
static void near(float value,float expected){assert(std::fabs(value-expected)<0.00001f);}
int main(){
    TVec3f hostZero{1,2,3};hostZero.setPSZeroVec();near(hostZero.x,0);near(hostZero.y,0);near(hostZero.z,0);
    TQuat4f quarterTurn;quarterTurn.setEulerDegree(0,0,90);TVec3f xAxis,yAxis;quarterTurn.getXDir(xAxis);quarterTurn.getYDir(yAxis);
    near(xAxis.x,0);near(xAxis.y,1);near(xAxis.z,0);near(yAxis.x,-1);near(yAxis.y,0);near(yAxis.z,0);
    Quaternion identity{0,0,0,3};Mtx matrix;PSMTXQuat(matrix,&identity);
    for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col)near(matrix[row][col],row==col?1:0);
    Quaternion zTurn{0,0,2,2};PSMTXQuat(matrix,&zTurn);
    near(matrix[0][0],0);near(matrix[0][1],-1);near(matrix[1][0],1);near(matrix[1][1],0);near(matrix[2][2],1);
    unsigned checked=0;
    for(int x=-2;x<=2;++x)for(int y=-2;y<=2;++y)for(int z=-2;z<=2;++z)for(int w=-2;w<=2;++w){
        if(!(x||y||z||w))continue;
        Quaternion q{float(x),float(y),float(z),float(w)},negative{-q.x,-q.y,-q.z,-q.w};
        Mtx opposite;PSMTXQuat(matrix,&q);PSMTXQuat(opposite,&negative);
        for(unsigned row=0;row<3;++row){near(matrix[row][3],0);
            for(unsigned col=0;col<3;++col){near(matrix[row][col],opposite[row][col]);
                float dot=0;for(unsigned k=0;k<3;++k)dot+=matrix[row][k]*matrix[col][k];near(dot,row==col?1:0);
            }
        }
        float determinant=matrix[0][0]*(matrix[1][1]*matrix[2][2]-matrix[1][2]*matrix[2][1])
          -matrix[0][1]*(matrix[1][0]*matrix[2][2]-matrix[1][2]*matrix[2][0])
          +matrix[0][2]*(matrix[1][0]*matrix[2][1]-matrix[1][1]*matrix[2][0]);near(determinant,1);++checked;
    }
    std::printf("PASS host Euler-degree and paired-single zero compatibility; %u quaternion rotations, sign equivalence, orthogonality, determinant and translation\n",checked);
}
