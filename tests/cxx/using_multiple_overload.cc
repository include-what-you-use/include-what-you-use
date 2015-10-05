//===--- using_multiple_overload.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we use multiple function overloads through a using decl, 
// that we correctly include all of the necessary files for the overload and
// don't accidently remove files greedily.

#include <vector>
#include <list>

void use_overload() {  
  std::vector<int> a(10);
  std::vector<int> b(11);
  std::list<float> c;
  std::list<float> d;
  using std::swap;
  swap(a, b);
  swap(c, d);
}

/**** IWYU_SUMMARY

(tests/cxx/using_multiple_overload.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
