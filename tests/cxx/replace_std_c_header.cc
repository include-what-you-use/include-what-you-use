//===--- replace_std_c_header.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests replacing C standard library headers with the C++ counterparts.
// The behavior was proposed in
// https://github.com/include-what-you-use/include-what-you-use/issues/1667.

#include <stdlib.h>

int main() {
  // IWYU: EXIT_SUCCESS is...*<cstdlib>
  return EXIT_SUCCESS;
}

/**** IWYU_SUMMARY

tests/cxx/replace_std_c_header.cc should add these lines:
#include <cstdlib>

tests/cxx/replace_std_c_header.cc should remove these lines:
- #include <stdlib.h>  // lines XX-XX

The full include-list for tests/cxx/replace_std_c_header.cc:
#include <cstdlib>  // for EXIT_SUCCESS

***** IWYU_SUMMARY */
