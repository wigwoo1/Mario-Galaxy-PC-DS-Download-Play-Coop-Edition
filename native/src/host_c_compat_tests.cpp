#include <cassert>
#include <climits>

extern "C" int __abs(int value);

int main() {
    assert(__abs(0) == 0);
    assert(__abs(1) == 1);
    assert(__abs(-1) == 1);
    assert(__abs(12345) == 12345);
    assert(__abs(-12345) == 12345);
    assert(__abs(INT_MIN) == INT_MIN);
    assert(__abs(INT_MAX) == INT_MAX);
    return 0;
}
