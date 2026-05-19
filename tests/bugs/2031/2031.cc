//===--- 2031.cc ----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "2031.h"
#include <iostream>
#include <string>

std::string GetGreeting(const std::string& name) {
  return "Hello, " + name + "!";
}

int main() {
  // std::string should be reported here, or possibly at the GetGreeting
  // definition. But the macro expansion confuses IWYU.
  std::string greeting = GetGreeting(BOB);
  std::cout << greeting << std::endl;
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/2031/2031.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
