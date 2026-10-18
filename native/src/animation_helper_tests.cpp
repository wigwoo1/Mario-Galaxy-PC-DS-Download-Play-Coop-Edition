#include <Game/Util/MtxUtil.hpp>
#include <Game/Util/MathUtil.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <initializer_list>
namespace MR {void extractString(char*,const char*,u32,u32);}
int main(){
 TVec3f from(4,-8,16),to(-12,24,0),result;
 for(float rate:{-0.5f,0.0f,0.25f,0.5f,1.0f,1.5f}){
   MR::vecBlend(from,to,&result,rate);
   assert(result.x==4-16*rate && result.y==-8+32*rate && result.z==16-16*rate);
   auto aliasedFrom=from,aliasedTo=to;
   MR::vecBlend(aliasedFrom,to,&aliasedFrom,rate);
   MR::vecBlend(from,aliasedTo,&aliasedTo,rate);
   assert(aliasedFrom.x==result.x && aliasedFrom.y==result.y && aliasedFrom.z==result.z);
   assert(aliasedTo.x==result.x && aliasedTo.y==result.y && aliasedTo.z==result.z);
 }
 MR::PSvecBlend(&from,&to,&result,2,3);
 assert(result.x==-28 && result.y==56 && result.z==32);
 Mtx matrix={{2,3,4,5},{6,7,8,9},{10,11,12,13}};
 MR::setMtxTrans(matrix,-15,0.25f,100);TVec3f position;MR::extractMtxTrans(matrix,&position);
 assert(position.x==-15 && position.y==0.25f && position.z==100);
 assert(matrix[0][0]==2 && matrix[0][1]==3 && matrix[0][2]==4 && matrix[1][0]==6 && matrix[2][2]==12);
 char text[9];std::memset(text,'!',sizeof(text));MR::extractString(text,"run/walk",3,99);
 assert(!std::strcmp(text,"run") && text[4]=='!');
 MR::extractString(text,"a",4,0);assert(text[0]=='a' && text[1]==0 && text[3]==0 && text[4]==0);
 MR::extractString(text,"run",0,0);assert(text[0]==0);
 std::puts("PASS original matrix translation, preserved rotation/scale, string prefix/padding/termination");
}
