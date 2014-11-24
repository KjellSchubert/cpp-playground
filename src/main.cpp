#include <iostream>
#include <exception>

// this import* is frowned upon, but this is a toy project and I dont wanna import used classes 1 by 1
using namespace std;

// pull in main entry points from the other files in src/
void play_with_cpp11();
void play_with_locale();
void play_with_move();
void play_with_stl();
void play_with_threading();
void variadic_template_func();

int main() {
  try {
		cout << "entering main()" << endl;
    play_with_cpp11();
    play_with_locale();
    variadic_template_func();
    play_with_move();
    play_with_stl();
    play_with_threading();
		cout << "exiting main()" << endl;
  }
  catch (const std::exception& ex) {
    cout << "caught exception: " << ex.what() << endl;
  }
  return 0;
} 
