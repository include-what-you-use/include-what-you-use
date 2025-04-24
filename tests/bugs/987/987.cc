//===--- 987.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include "987.h"

// This is already included in 987.h, and so could be removed here.
#include <cstdint>

void foo(std::uint8_t arg) {
}

/**** IWYU_SUMMARY

tests/bugs/987/987.cc should add these lines:

tests/bugs/987/987.cc should remove these lines:
- #include <cstdint>  // lines XX-XX

The full include-list for tests/bugs/987/987.cc:

***** IWYU_SUMMARY */
