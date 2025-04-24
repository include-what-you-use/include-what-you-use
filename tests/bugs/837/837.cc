//===--- 837.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "fooa.h"
#include "foob.h"

int main() {
  auto pFooB{getPointerFooB()};

  // std::unique_ptr<fooB> pFooB{ getPointerFooB()};
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/837/837.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
