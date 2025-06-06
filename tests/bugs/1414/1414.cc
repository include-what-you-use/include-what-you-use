//===--- 1414.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++20
// IWYU_XFAIL

#include <concepts>
#include "derived.h"

class Base;

template <typename T, typename U>
  requires std::derived_from<T, U>
class C {};

int main() {
  C<Derived, Base>();
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/1414/1414.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
