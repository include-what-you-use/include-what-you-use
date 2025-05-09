//===--- 752.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
// IWYU_ARGS: -std=c++2a

#include "compat.h"

int main() {
  compat::optional<int> o;
}

/**** IWYU_SUMMARY

(tests/bugs/752/752.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
