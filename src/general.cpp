#include <iostream>
#include <type_traits>

using namespace std;

// general C++ tests, not specific to C++11
void play_general() {
    
  {
    struct A { 
        int x = 7;
        virtual ~A() {}
    };
    struct B : public A {
        //int y = 8; // not enough on some machines
        int y[4] = {1,2,3,4};
    };
    static_assert(sizeof(A) != sizeof(B), ""); // not guaranteed
    
    // Now this here is a terrible example of type-safety gone wrong:
    // The B[3] can be passed to a func taking type A[3], effectively 
    // silenty reinterpret-casting memory locations to B*.
    // I knew C++ (due to its C legacy) lets T[] decay to a T* (which is pretty
    // bad alrdy), but I didn't think this allows decaying B[3] to A[3].
    // See http://stackoverflow.com/questions/12064333/assigning-derived-class-array-to-base-class-pointer
    // This problem can be avoided by using std::array<A, 3> btw.
    B bs[3]; // = {}
    auto funcTakingAs = [](A as[3]) { 
        // likely crash here when called with B[] 
        bool allAre7 = true;
        for (int i=0; i<3; ++i) {
            if (as[i].x != 7)
                allAre7 = false;
            cout << "xxx " << as[i].x << '\n';
        }
        return allAre7;
    };
    bool allAre7 = funcTakingAs(bs);
    
    // This here surprising compiles also, same array/pointer decay reason:
    auto funcTaking4Bs = [](B bs[4]) {};
    funcTaking4Bs(bs);
  }

}