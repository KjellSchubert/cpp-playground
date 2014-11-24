#include <iostream>
#include <iomanip>
#include <locale>
#include <cassert>
#include <string>
#include <sstream> // <strstream> is deprecated
#include <fstream>

//#include <codecvt> is this missing in gcc 4.9 c++lib???
// See https://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html#status.iso.2011
 
using namespace std;

std::wstring utf8_to_utf16(std::string str) { // or use u16string?
  // this would be simpler with http://en.cppreference.com/w/cpp/locale/codecvt_utf8
  
  /* couldnt get this to work with strstreams
  std::istringstream iss(str);
  iss.imbue(std::locale("en_US.UTF8"));
  for (wchar_t c; iss >> c; ) // is u16_char_t
     std::cout << "U+" << std::hex << std::setw(4) << std::setfill('0') << c << '\n';
  return TODO; 
  */

  // this impl here is ridiculous, since it goes thru files:
  std::ofstream("text.txt") << str; // writes utf8 byte seq
  std::wifstream fin("text.txt");
  fin.imbue(std::locale("en_US.UTF-8"));
  for (wchar_t c; fin >> c; )
     std::cout << "U+" << std::hex << std::setw(4) << std::setfill('0') << c << '\n';
  return L"";
}

void play_with_locale() {
  // see http://en.cppreference.com/w/cpp/language/string_literal
  // for unicode string literals
  std::string strUtf8 = u8"\u00c4 10 \u20AC"; // A-umlaut & euro symbol. And notice the 'u8', which is C++11
  cout << "utf8 stream with euro symbol: " << strUtf8 
       << " len=" << strUtf8.length() << '\n';
  assert(strUtf8.length() == 9); // so the A-umlaut & euro symbol is 2-3 bytes long in utf8
  // note the euro symbol is [e2 82 ac] in utf8

  // convert utf8 -> utf16
  /* doesn't work yet, TODO
  std::wstring strW = utf8_to_utf16(strUtf8);
  assert(strW.length() == 6); // number of code points aka utf16 chars
  */
  //
  // TODO std::wstring data = 
  // TODO std::isupper(c, locale)
}
