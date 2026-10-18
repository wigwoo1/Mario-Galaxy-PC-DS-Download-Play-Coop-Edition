#pragma once
namespace Native {
// Shared integer combiner arithmetic for native material pixel shaders.
// Signed HLSL division truncates; TEV requires arithmetic floor division.
inline constexpr const char* tevArithmeticHlsl=R"(
int tevFloorDivide(int v,int d){return v>=0?v/d:-((-v+d-1)/d);}
int tevArithmetic(int a,int b,int c,int d,uint bias,bool sub,uint scale,bool clampResult){
 int weight=c+c/128;
 int interpolation=a*256+(b-a)*weight;
 int base=d+(bias==1?128:bias==2?-128:0);
 if(scale!=3){int factor=1<<scale;interpolation*=factor;base*=factor;interpolation+=sub?127:128;}
 int value=base+(sub?-1:1)*tevFloorDivide(interpolation,256);
 if(scale==3)value=tevFloorDivide(value,2);
 return clamp(value,clampResult?0:-1024,clampResult?255:1023);
}
bool tevPackedCompare(int3 a,int3 b,uint op){
 uint lhs=uint(a.x),rhs=uint(b.x);
 if(op>=2){lhs|=uint(a.y)<<8;rhs|=uint(b.y)<<8;}
 if(op>=4){lhs|=uint(a.z)<<16;rhs|=uint(b.z)<<16;}
 return (op&1)!=0?lhs==rhs:lhs>rhs;
}
int3 tevColorCompare(int3 a,int3 b,int3 c,int3 d,uint op,bool clampResult){
 bool3 condition;
 if(op<6){bool packed=tevPackedCompare(a,b,op);condition=bool3(packed,packed,packed);}
 else condition=op==6?a>b:a==b;
 int3 value=d+int3(condition.x?c.x:0,condition.y?c.y:0,condition.z?c.z:0);
 return clamp(value,clampResult?0:-1024,clampResult?255:1023);
}
int tevAlphaCompare(int3 ca,int3 cb,int a,int b,int c,int d,uint op,bool clampResult){
 bool condition=op<6?tevPackedCompare(ca,cb,op):op==6?a>b:a==b;
 return clamp(d+(condition?c:0),clampResult?0:-1024,clampResult?255:1023);
}
)";
}
