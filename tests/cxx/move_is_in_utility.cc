//===--- move_is_in_utility.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// std::move comes from <utility>, not <algorithm>.

#include <utility>
#include <algorithm>

int foo(int x)
{
  int k = 3;
  return std::move(k);
}


/**** IWYU_SUMMARY
tests/cxx/move_is_in_utility.cc should add these lines:

tests/cxx/move_is_in_utility.cc should remove these lines:
- #include <algorithm>  // lines XX-XX

The full include-list for tests/cxx/move_is_in_utility.cc:
#include <utility>  // for move
***** IWYU_SUMMARY */
