//===--- std_size_t.cc - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This tests a little quirk with the mappings for the new <cstd...> headers.
// size_t is such a commonly-used type that it deserves a dedicated test to
// demonstrate how this behaves in general. Note how the use of 'printf' is
// attributed to <cstdio> and <stdio.h> is removed. This happens because
// std::size_t is mapped to <cstdio> before 'printf' is seen and already-needed
// includes take precedence if a symbol is available from multiple sources.
//
// (Note that if 'printf' was seen before std::size_t, <stdio.h> would still be
// required.)

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
