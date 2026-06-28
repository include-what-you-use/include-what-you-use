//===--- std_size_t.cc - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// In this test, IWYU picks <cstdio> for printf and size_t although it is
// not guaranteed that <c...> headers provide names from the global namespace.

#include <cstdio>
#include <stdio.h>

std::size_t f() {
  const size_t x = 100;
  printf("%zu\n", x);
  return sizeof(int);
}

/**** IWYU_SUMMARY

tests/cxx/std_size_t.cc should add these lines:

tests/cxx/std_size_t.cc should remove these lines:
- #include <stdio.h>  // lines XX-XX

The full include-list for tests/cxx/std_size_t.cc:
#include <cstdio>  // for printf, size_t

***** IWYU_SUMMARY */
