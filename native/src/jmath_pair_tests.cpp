#include <utility>
#include "JSystem/JMath/JMATrigonometric.hpp"
#include <cstddef>
#include <type_traits>
static_assert(sizeof(JMath::TrigTablePair<float,float>)==8);
static_assert(std::is_standard_layout<JMath::TrigTablePair<float,float>>::value);
using TrigPair=JMath::TrigTablePair<float,float>;
static_assert(offsetof(TrigPair,a1)==0 && offsetof(TrigPair,b1)==4);
static_assert(sizeof(JMath::TSinCosTable<14,float>)==16384*8);
int main(){std::pair<int,int> standard{4,7}; TrigPair cell; if(cell.a1!=0 || cell.b1!=0)return 1; cell.a1=2;cell.b1=-3;return standard.first==4 && standard.second==7 && cell.a1==2 && cell.b1==-3 ? 0:2;}
