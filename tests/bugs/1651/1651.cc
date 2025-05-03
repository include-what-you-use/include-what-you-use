//===--- 1651.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -stdlib=libstdc++
// IWYU_XFAIL
#include "1651.h"

#include <iostream>
#include <string>

void f(const S& s) {
  std::cout << s.str;
}

/**** IWYU_SUMMARY

tests/bugs/1651/1651.cc should add these lines:

tests/bugs/1651/1651.cc should remove these lines:
- #include <string>  // lines XX-XX

The full include-list for tests/bugs/1651/1651.cc:
#include "1651.h"
#include <iostream>  // for basic_ostream, operator<<, cout

***** IWYU_SUMMARY */
