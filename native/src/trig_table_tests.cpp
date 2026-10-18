#include <JSystem/JMath/JMATrigonometric.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
    const float pi=3.14159265358979323846f;
    for(int i=0;i<1024;++i){assert(std::fabs(JMath::sAsinAcosTable.mTable[i]-std::asin(double(i)/1024))<.000001);assert(std::fabs(JMath::sAtanTable.mTable[i]-std::atan(double(i)/1024))<.000001);}
    assert(JMath::sAsinAcosTable.asin_(0)==0);
    assert(std::fabs(JMath::sAsinAcosTable.asin_(1)-pi/2)<.000001f&&std::fabs(JMath::sAsinAcosTable.acos_(-1)-pi)<.000001f);
    for(int y=-3;y<=3;++y)for(int x=-3;x<=3;++x)if(x||y)assert(std::fabs(JMath::sAtanTable.atan2_(float(y),float(x))-std::atan2(double(y),double(x)))<.001f);
    std::puts("PASS original trig table initialization, endpoints and atan2 quadrants");
}

