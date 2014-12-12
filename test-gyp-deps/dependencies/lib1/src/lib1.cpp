#include "lib1/lib.h"
#include "lib2/lib.h"

int lib1_func(const std::string& bla) {
    lib2::SomeType len = bla.length();
    return len;
}