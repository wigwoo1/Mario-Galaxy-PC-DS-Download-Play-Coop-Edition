#include "Game/Util/BothDirList.hpp"
#include <cstdio>
#include <stdexcept>
static void check(bool ok) { if (!ok) throw std::runtime_error("Original runtime list ownership failed"); }
int main() {
  MR::BothDirPtrList a(true), b(true);
  MR::BothDirPtrLink first(nullptr), middle(nullptr), last(nullptr);
  check(a.append(&first) && a.append(&middle) && a.append(&last));
  check(a.mCount==3 && a.mHead==&first && a.mTail==&last);
  check(b.append(&middle));
  check(a.mCount==2 && first.mNextLink==&last && last.mPrevLink==&first);
  check(b.mCount==1 && middle.mList==&b);
  check(!a.remove(&middle) && b.mCount==1);
  check(a.append(&first));
  check(a.mHead==&last && a.mTail==&first && a.mCount==2);
  {
    MR::BothDirPtrLink temporary(nullptr);
    check(a.append(&temporary) && a.mCount==3);
  }
  check(a.mCount==2 && a.mTail==&first && first.mNextLink==nullptr);
  check(a.remove(&last) && a.mHead==&first && first.mPrevLink==nullptr);
  check(a.remove(&first) && a.mCount==0 && !a.mHead && !a.mTail);
  MR::BothDirPtrLink survivor(nullptr);
  {
    MR::BothDirPtrList owner(true);
    check(owner.append(&survivor));
  }
  check(survivor.mList==nullptr && b.append(&survivor) && b.mCount==2);
  std::puts("PASS original runtime list transfer, reorder, removals, automatic unlink and owner destruction");
}
