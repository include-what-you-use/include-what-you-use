//===--- std_size_t.cc - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// In this test, IWYU should pick <stdio.h> for printf and size_t because it is
// not guaranteed that <c...> headers provide names from the global namespace.
// Qualified std::size_t is attributed to <cstddef>.

#include <cstdio>
#include <stdio.h>

// IWYU: std::size_t is...*<cstddef>
std::size_t f() {
  const size_t x = 100;
  printf("%zu\n", x);
  return sizeof(int);
}

/**** IWYU_SUMMARY

tests/cxx/std_size_t.cc should add these lines:
#include <cstddef>

tests/cxx/std_size_t.cc should remove these lines:
- #include <cstdio>  // lines XX-XX

The full include-list for tests/cxx/std_size_t.cc:
#include <stdio.h>  // for printf, size_t
#include <cstddef>  // for size_t

***** IWYU_SUMMARY */
