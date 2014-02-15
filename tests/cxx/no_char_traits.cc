//===--- no_char_traits.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A very small test that makes sure we don't suggest char_traits
// when using ostreams with a char*.  That triggers this code in
// <ostream>:
//   inline basic_ostream<char, _Traits>&
//   operator<<(basic_ostream<char, _Traits>& __out, const char* __s)
//   {
//     if (!__s)
//       __out.setstate(ios_base::badbit);
//   else
//       __ostream_insert(__out, __s,
//                        static_cast<streamsize>(_Traits::length(__s)));
//     return __out;
//   }
// But for char*'s <ostream> should intend-to-provide traits, even
// though it's a template arg, so the user doesn't have to.

#include <iostream>

int main() {
  std::cout << "Hello, world.\n";
  // This is more difficult because the first << returns a basic_ostream.
  std::cout << "Hello, world." << "\n";
}

/**** IWYU_SUMMARY

(tests/cxx/no_char_traits.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
