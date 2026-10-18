#include <JSystem/JSupport/JSUList.hpp>
#include <cassert>
#include <cstdio>
int main(){
 int a=1,b=2,c=3; JSUPtrLink x(&a),y(&b),z(&c);
 JSUPtrList first,second;
 assert(first.append(&x)); assert(first.append(&y)); assert(first.prepend(&z));
 assert(z.getNext()==&x && x.getPrev()==&z && x.getNext()==&y);
 assert(second.append(&x)); assert(x.getList()==&second);
 assert(z.getNext()==&y && y.getPrev()==&z);
 assert(!first.remove(&x)); assert(second.remove(&x)); assert(x.getList()==nullptr);
 { JSUPtrLink temporary(&a); assert(first.append(&temporary)); }
 assert(y.getNext()==nullptr);
 { JSUPtrList temporary; assert(temporary.append(&x)); }
 assert(x.getList()==nullptr);
 assert(first.remove(&z)); assert(first.remove(&y));
 std::puts("Original JSUList PASS: order, transfer, membership checks, link and list destruction");
}
