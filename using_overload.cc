//===--- using_overload.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <vector>
#include <list>

void use_overload() {  
  std::vector<int> a(10);
  std::vector<int> b(11);
  using std::swap;
  swap(a, b);
}

/**** IWYU_SUMMARY

tests/cxx/using_overload.cc should add these lines:

tests/cxx/using_overload.cc should remove these lines:
- #include <list>  // lines XX-XX

The full include-list for tests/cxx/using_overload.cc:
#include <vector>  // for swap, vector

***** IWYU_SUMMARY */
