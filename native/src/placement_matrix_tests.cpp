#include <revolution/mtx.h>
namespace JMathInlineVEC { float PSVECSquareDistance(const Vec*,const Vec*); }
#include <cassert>
#include <Game/Util/MtxUtil.hpp>
#include <cmath>
#include <cstdio>
#include <stdexcept>
static void close(float actual,float expected){if(std::abs(actual-expected)>0.002f)throw std::runtime_error("placement matrix mismatch");}
int main(){
 try {
 Vec magnitudeInput{3,-4,12};close(C_VECMag(&magnitudeInput),13);Vec magnitudeZero{0,0,0};close(C_VECMag(&magnitudeZero),0);assert(magnitudeInput.x==3 && magnitudeInput.y==-4 && magnitudeInput.z==12);
 TQuat4f axisAngle;axisAngle.setRotate(TVec3f(0,0,1),1.57079632679f);
 TVec3f axisRotated(2,0,3);axisAngle.transform(axisRotated);
 close(axisRotated.x,0);close(axisRotated.y,2);close(axisRotated.z,3);
 axisAngle.setRotate(TVec3f(0,2,0),1.57079632679f);
 close(axisAngle.x,0);close(axisAngle.y,1.41421356237f);close(axisAngle.z,0);close(axisAngle.w,0.70710678118f);
 axisAngle.setRotate(TVec3f(1,0,0),0);close(axisAngle.x,0);close(axisAngle.y,0);close(axisAngle.z,0);close(axisAngle.w,1);
 // Non-unit input and antiparallel direction tests exercise angle without normalization assumptions.
 close(TVec3f(2,0,0).angle(TVec3f(0,7,0)),1.57079632679f);
 close(TVec3f(2,0,0).angle(TVec3f(-7,0,0)),3.14159265359f);
 close(TVec3f(2,0,0).angle(TVec3f(7,0,0)),0);
 TQuat4f inplaceQ;inplaceQ.setEuler(0,0,1.57079632679f);
 TVec3f inplaceVector(2,0,3);inplaceQ.transform(inplaceVector);
 close(inplaceVector.x,0);close(inplaceVector.y,2);close(inplaceVector.z,3);
 TPos3f basis;basis.setTR(TVec3f(1,2,3),TVec3f(4,5,6),TVec3f(7,8,9),TVec3f(10,11,12));const float basisExpected[3][4]={{1,4,7,10},{2,5,8,11},{3,6,9,12}};for(int r=0;r<3;++r)for(int c=0;c<4;++c)close(basis.mMtx[r][c],basisExpected[r][c]);
 TVec3f extractedZ;basis.getZDir2(extractedZ);close(extractedZ.x,7);close(extractedZ.y,8);close(extractedZ.z,9);TVec3f extractedY;basis.getYDir2(extractedY);close(extractedY.x,4);close(extractedY.y,5);close(extractedY.z,6);
 TPos3f directions;directions.identity();directions.setTrans(3,4,5);
 directions.setRotate(TVec3f(1,0,0),TVec3f(0,1,0));
 const float directionExpected[3][4]={{0,-1,0,3},{1,0,0,4},{0,0,1,5}};
 for(int r=0;r<3;r++)for(int c=0;c<4;c++)close(directions.mMtx[r][c],directionExpected[r][c]);
 // Preserve the original degenerate cross-product rule, including opposite directions.
 directions.setRotate(TVec3f(1,0,0),TVec3f(-1,0,0));
 for(int r=0;r<3;r++)for(int c=0;c<3;c++)close(directions.mMtx[r][c],r==c?1:0);
 directions.setRotate(TVec3f(0,0,0),TVec3f(1,0,0));
 for(int r=0;r<3;r++)for(int c=0;c<3;c++)close(directions.mMtx[r][c],r==c?1:0);
 for(const auto axis: {TVec3f(180,0,0),TVec3f(0,180,0),TVec3f(0,0,180),TVec3f(20,30,40)}){
  TPos3f source,roundtrip;source.identity();source.setRotate(axis*0.0174532925199f);TQuat4f quaternion;source.getQuat(quaternion);roundtrip.identity();roundtrip.setQuat(quaternion);
  for(int r=0;r<3;r++)for(int c=0;c<3;c++)close(roundtrip.mMtx[r][c],source.mMtx[r][c]);
 }
 TQuat4f halfTurn;halfTurn.setRotate(TVec3f(1,0,0),TVec3f(0,1,0),0.5f);TVec3f halfway;halfTurn.getXDir(halfway);close(halfway.x,0.70710678f);close(halfway.y,0.70710678f);close(halfway.z,0);
 TVec3f projected(3,4,5);projected.orthogonalize(TVec3f(0,1,0));close(projected.x,3);close(projected.y,0);close(projected.z,5);TVec3f alias(0,1,0);alias.orthogonalize(alias);close(alias.length(),0);
 Mtx matrix;
 TPos3f radians;radians.identity();radians.mMtx[0][3]=11;radians.mMtx[1][3]=22;radians.mMtx[2][3]=33;
 radians.setRotate(TVec3f(0,0,1.57079632679f));
 const float radiansExpected[3][4]={{0,-1,0,11},{1,0,0,22},{0,0,1,33}};
 for(int r=0;r<3;r++)for(int c=0;c<4;c++)close(radians.mMtx[r][c],radiansExpected[r][c]);
 MR::makeMtxTR(matrix,11,22,33,0,0,90);
 const float expected[3][4]={{0,-1,0,11},{1,0,0,22},{0,0,1,33}};
 for(int r=0;r<3;r++)for(int c=0;c<4;c++)close(matrix[r][c],expected[r][c]);
 MR::makeMtxRotate(matrix,0.0f,90.0f,0.0f);
 const float rotation[3][4]={{0,0,1,0},{0,1,0,0},{-1,0,0,0}};
 for(int r=0;r<3;r++)for(int c=0;c<4;c++)close(matrix[r][c],rotation[r][c]);
 MR::makeMtxTRS(matrix,11,22,33,0,0,90,2,3,-4);
 const float scaled[3][4]={{0,-3,0,11},{2,0,0,22},{0,0,-4,33}};
 for(int r=0;r<3;r++)for(int c=0;c<4;c++)close(matrix[r][c],scaled[r][c]);
 MR::makeMtxTransRotateY(matrix,11,22,33,90);
 for(int r=0;r<3;r++)for(int c=0;c<4;c++)close(matrix[r][c],c==3?float((r+1)*11):rotation[r][c]);
 auto x=MR::tmpMtxRotXDeg(90);close(x[1][2],-1);close(x[2][1],1);
 auto y=MR::tmpMtxRotYDeg(-90);close(y[0][2],-1);close(y[2][0],1);
 auto z=MR::tmpMtxRotZDeg(180);close(z[0][0],-1);close(z[1][1],-1);
 x=MR::tmpMtxRotXDeg(0);for(int r=0;r<3;r++)for(int c=0;c<4;c++)close(x[r][c],r==c?1:0);
 {Mtx source={{1,0,0,7},{0,1,0,8},{0,0,1,9}};TPos3f matrix=source;matrix.setEulerZ(3.14159265358979323846f/2);close(matrix.mMtx[0][0],0);close(matrix.mMtx[0][1],-1);close(matrix.mMtx[1][0],1);close(matrix.mMtx[1][1],0);close(matrix.mMtx[2][2],1);close(matrix.mMtx[0][3],7);close(matrix.mMtx[1][3],8);close(matrix.mMtx[2][3],9);matrix.setRotateDegree(TVec3f(90,0,0));close(matrix.mMtx[1][2],-1);close(matrix.mMtx[2][1],1);close(matrix.mMtx[0][3],7);TVec3f v;v.setAll<float>(2.5f);close(v.x,2.5f);close(v.y,2.5f);close(v.z,2.5f);}{Mtx transform={{2,3,4,101},{-1,5,2,202},{7,0,-3,303}};TVec3f input[3]={TVec3f(1,2,3),TVec3f(-4,5,-6),TVec3f(0,0,0)},output[3];float expected[3][3];for(int i=0;i<3;++i){const float values[]={input[i].x,input[i].y,input[i].z};for(int r=0;r<3;++r){expected[i][r]=0;for(int c=0;c<3;++c)expected[i][r]+=transform[r][c]*values[c];}Vec sdk;PSMTXMultVecSR(transform,&input[i],&sdk);close(sdk.x,expected[i][0]);close(sdk.y,expected[i][1]);close(sdk.z,expected[i][2]);}PSMTXMultVecArraySR(transform,input,output,3);for(int i=0;i<3;++i){close(output[i].x,expected[i][0]);close(output[i].y,expected[i][1]);close(output[i].z,expected[i][2]);}PSMTXMultVecArraySR(transform,input,input,3);for(int i=0;i<3;++i){close(input[i].x,expected[i][0]);close(input[i].y,expected[i][1]);close(input[i].z,expected[i][2]);}PSMTXMultVecArraySR(transform,static_cast<const TVec3f*>(nullptr),static_cast<TVec3f*>(nullptr),0);}{TQuat4f q(0.0f,0.0f,std::sqrt(0.5f),std::sqrt(0.5f));TVec3f v(2,3,4),out;q.transform(v,out);close(out.x,-3);close(out.y,2);close(out.z,4);q.rotate(v);close(v.x,out.x);close(v.y,out.y);close(v.z,out.z);TPos3f matrix;matrix.setTrans(9,8,7);matrix.makeRotate(TVec3f(0,2,0),3.14159265358979323846f/2);close(matrix.mMtx[0][2],1);close(matrix.mMtx[2][0],-1);close(matrix.mMtx[1][1],1);for(int r=0;r<3;++r)close(matrix.mMtx[r][3],0);}{TPos3f matrix;matrix.setTrans(9,8,7);matrix.setRotate(TVec3f(0,2,0),3.14159265358979323846f/2);close(matrix.mMtx[0][0],0);close(matrix.mMtx[0][2],1);close(matrix.mMtx[2][0],-1);close(matrix.mMtx[1][1],1);close(matrix.mMtx[0][3],9);close(matrix.mMtx[1][3],8);close(matrix.mMtx[2][3],7);}{JGeometry::TProjection3<JGeometry::TMatrix44<JGeometry::SMatrix44C<float>>> projection;projection.identity();auto matrix=projection.toMtx44();assert(matrix==projection.mMtx);for(int r=0;r<4;++r)for(int c=0;c<4;++c){matrix[r][c]=float(r*4+c+1);close(projection.mMtx[r][c],float(r*4+c+1));}}{TPos3f matrix;for(int r=0;r<3;++r)for(int c=0;c<4;++c)matrix.mMtx[r][c]=-77.0f;matrix.makeTrans(-3,4,12);for(int r=0;r<3;++r)for(int c=0;c<3;++c)close(matrix.mMtx[r][c],r==c?1.0f:0.0f);close(matrix.mMtx[0][3],-3);close(matrix.mMtx[1][3],4);close(matrix.mMtx[2][3],12);TVec3f input(5,-6,7),out;matrix.mult(input,out);close(out.x,2);close(out.y,-2);close(out.z,19);matrix.makeTrans(TVec3f(8,-9,10));matrix.mult(input,out);close(out.x,13);close(out.y,-15);close(out.z,17);for(int r=0;r<3;++r)for(int c=0;c<3;++c)close(matrix.mMtx[r][c],r==c?1.0f:0.0f);TVec2f a(-2,5),b(1,9);close(a.distance(b),5);close(b.distance(a),5);close(a.distance(a),0);TVec2f corner(608,456),origin(0,0);close(corner.distance(origin),760);close(a.x,-2);close(a.y,5);close(b.x,1);close(b.y,9);}{TQuat4f identity(0.0f,0.0f,0.0f,1.0f),halfTurn(0.0f,0.0f,1.0f,0.0f),q;q.slerp(identity,halfTurn,0.5f);TVec3f v(1,0,0);q.rotate(v);close(v.x,0);close(v.y,1);close(v.z,0);q.slerp(identity,halfTurn,0.0f);close(q.w,1);q.slerp(identity,halfTurn,1.0f);close(q.z,1);TQuat4f negative(0.0f,0.0f,0.0f,-1.0f);q.slerp(identity,negative,0.5f);close(q.w,1);TPos3f matrix;matrix.setTrans(7,8,9);matrix.makeQuat(q);for(int r=0;r<3;++r){close(matrix.mMtx[r][3],0);for(int c=0;c<3;++c)close(matrix.mMtx[r][c],r==c?1.0f:0.0f);}}{TPos3f m;TVec3f axis;m.identity();close(m.getRotate(axis),0);close(axis.squared(),0);
 const float pi=3.14159265358979323846f;
 for(int component=0;component<3;++component){TVec3f direction(0,0,0);(&direction.x)[component]=1;
 for(float angle:{0.5f,pi}){m.identity();m.setRotate(direction,angle);float found=m.getRotate(axis);close(found,angle);close(axis.dot(direction),1);}}
 TQuat4f q(0,0,0,-1);close(q.getRotate(axis),0);close(axis.squared(),0);
 m.identity();m.setEulerZ(-0.5f);close(m.getRotate(axis),0.5f);close(axis.z,-1);
}{TVec3f direction(0,1,0),source(1,3,4);auto v=direction.getOrthogonal(source);close(v.x,1);close(v.y,0);close(v.z,4);direction.y=2;v=direction.getOrthogonal(source);close(v.y,-9);close(source.y,3);close(direction.y,2);}{Vec a{1,2,3},b{-2,6,-9};close(JMathInlineVEC::PSVECSquareDistance(&a,&b),169);close(JMathInlineVEC::PSVECSquareDistance(&b,&a),169);close(JMathInlineVEC::PSVECSquareDistance(&a,&a),0);close(a.x,1);close(b.z,-9);}{TVec3f x(1,0,0),y(0,1,0),z(0,0,1);assert(!z.orientation(x,y));assert(z.orientation(y,x));assert(!x.orientation(x,y));assert(!z.orientation(x,x));}std::puts("PASS original placement translation, axis rotations and scratch reset");return 0;
 }catch(const std::exception& e){std::puts(e.what());return 1;}
}

























