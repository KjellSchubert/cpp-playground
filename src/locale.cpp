#include <iostream>
#include <iomanip>
#include <locale>
#include <clocale> // std::setlocale
#include <cassert>
#include <string>
#include <sstream> // <strstream> is deprecated
#include <fstream>
#include <codecvt>

//#include <codecvt> is this missing in gcc 4.9 c++lib???
// See https://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html#status.iso.2011
 
using namespace std;

std::wstring utf8_to_utf16(std::string str) { // or use u16string?
  // this would be simpler with http://en.cppreference.com/w/cpp/locale/codecvt_utf8
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>,wchar_t> convert;
  return convert.from_bytes(str);
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
  wstring strW = utf8_to_utf16(strUtf8);
  assert(strW == L"\u00c4 10 \u20AC");

  //auto en_US_utf8 = std::locale("en_US.UTF-8");
  std::setlocale(LC_ALL, "en_US.UTF-8"); // not pretty that we have to call this just to be able to isUpper()
  //fails, needs German local for A-umlaut? assert(std::isupper(u'\u00c4'));
  assert(std::iswupper(u'\u00c4'));
}
