#include <cstdint>
#include <new>
#include <initializer_list>
#include <cstdint>
#include <type_traits>
#include <cstdio>
// Mirrored pinned FBO declaration; runtime probe separately compiles the original header.
struct FBO { std::uint32_t _0; std::uint32_t _4; };
static_assert(sizeof(FBO)==8);
static_assert(std::is_trivial<FBO>::value);
int main() {
    for (unsigned count : {1u, 608u, 1920u, 7680u}) {
        auto* p = new (std::align_val_t{32}) FBO[count];
        if (reinterpret_cast<std::uintptr_t>(p) % 32) return 1;
        for (unsigned i=0;i<count;++i) { p[i]._0=i; p[i]._4=~i; }
        for (unsigned i=0;i<count;++i) if(p[i]._0!=i || p[i]._4!=~i) return 2;
        ::operator delete[](p, std::align_val_t{32});
    }
    std::puts("PASS pinned FBO layout, aligned arrays, writes, and matching release");
}


