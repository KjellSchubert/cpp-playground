#include <iostream>
#include <stdexcept>
#include "main.h"
#include "lib1/lib.h"
#include "lib2/lib.h"

int main() {
    
    // this here verifies that lib1.gyp's #define was being inherited
    // by downstream targets
    int x = SOME_LIB1_DEFINE;
    if (x != 77)
        throw std::runtime_error("gyp problem? expected inherited SOME_LIB1_DEFINE=77");
        
    // this here verifies the lib2 #includes (for an #include-only lib) got
    // pulled in by gyp also:
    lib2::SomeType foo = 22;
        
    std::cout << "main " << StuffFromMain::thing << " " 
        << lib1_func("foo")
        << "\n";
}