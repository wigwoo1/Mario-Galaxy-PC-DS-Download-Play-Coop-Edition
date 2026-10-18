#include <nw4r/db/assert.h>

int main() {
    nw4r::db::Panic("nw4r-panic-test.cpp", 73, "bad %s=%d", "state", 9);
}
