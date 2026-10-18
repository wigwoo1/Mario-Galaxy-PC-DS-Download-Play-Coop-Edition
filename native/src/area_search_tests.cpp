#include <Native/AreaSearch.hpp>
#include <cassert>
#include <vector>
#include <cstdio>
int main(){
 int a=1,b=2,c=3;int* values[]={&a,&b,&c};std::vector<int> visited;
 auto contains=[&](int* p){visited.push_back(*p);return *p<=2;};
 assert(Native::findLastArea(values,3,contains)==&b);assert((visited==std::vector<int>{3,2}));
 visited.clear();assert(Native::findLastArea<int>(nullptr,0,contains)==nullptr&&visited.empty());
 assert(Native::findLastArea(values,3,[](int*){return true;})==&c);
 assert(Native::findLastArea(values,3,[](int* p){return *p==1;})==&a);
 assert(Native::findLastArea(values,3,[](int*){return false;})==nullptr);
 std::puts("PASS native area reverse search: empty, overlap priority, first-only, no match and predicate order");
}
