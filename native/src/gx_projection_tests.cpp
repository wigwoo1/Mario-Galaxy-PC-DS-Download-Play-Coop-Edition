#include <Native/GxCommandState.hpp>
#include "native_model_projection.hpp"
#include <revolution/gx/GXTransform.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
int main(){
 u8 bytes[33]{};GDLObj writer;GDInitGDLObj(&writer,bytes,33);GDSetCurrent(&writer);
 f32 matrix[4][4]={{2,0,.25f,7},{0,3,-.5f,8},{0,0,-1,-2},{0,0,-1,0}};
 GXSetProjection(matrix,GX_PERSPECTIVE);assert(GDGetCurrOffset()==33);
 Native::GxCommandState state;state.consume(bytes,33);auto p=state.projectionMatrix();
 assert(p[0][0]==2 && p[0][2]==.25f && p[0][3]==0 && p[1][2]==-.5f && p[2][3]==-2 && p[3][2]==-1 && p[3][3]==0);
 float retained[7];GXGetProjectionv(retained);assert(retained[0]==0&&retained[2]==.25f);
 GDInitGDLObj(&writer,bytes,33);GXSetProjection(matrix,GX_ORTHOGRAPHIC);state.consume(bytes,33);p=state.projectionMatrix();
 assert(p[0][3]==7&&p[1][3]==8&&p[0][2]==0&&p[1][2]==0&&p[3][2]==0&&p[3][3]==1);
 GDInitGDLObj(&writer,bytes,33);float vector[7]={0,4,.5f,5,-.25f,-2,-3};GXSetProjectionv(vector);state.consume(bytes,33);p=state.projectionMatrix();
 assert(p[0][0]==4&&p[0][2]==.5f&&p[1][1]==5&&p[2][3]==-3);
 std::vector<NativeShapeVertex> vertices(3);for(auto& v:vertices)v.position={1,2,-4};
 auto projected=projectModelTriangles(vertices,{{0,1,2}},state,{1,1,1,1});assert(projected.size()==3);
 assert(projected[0].position[0]==2&&projected[0].position[1]==11&&projected[0].position[2]==5&&projected[0].position[3]==4);
 GDInitGDLObj(&writer,bytes,32);bool rejected=false;try{GXSetProjection(matrix,GX_ORTHOGRAPHIC);}catch(const std::exception&){rejected=true;}
 GXGetProjectionv(retained);assert(rejected&&GDGetCurrOffset()==0&&retained[0]==0&&retained[1]==4);
 Native::GxCommandState empty;rejected=false;try{empty.projectionMatrix();}catch(const std::exception&){rejected=true;}assert(rejected);
 GDSetCurrent(nullptr);std::puts("PASS GX perspective/orthographic/vector projection command decoding and atomic capacity rejection");
}

