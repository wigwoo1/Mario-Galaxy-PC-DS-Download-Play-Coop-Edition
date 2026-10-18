#include <stdexcept>
static void check(bool value){if(!value)throw std::runtime_error("Hash table regression failed");}
#include "hash_sort_table_tests.hpp"
int main(){hashSortTableTests();}
