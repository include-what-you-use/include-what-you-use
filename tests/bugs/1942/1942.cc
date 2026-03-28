//===--- 1942.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "1942.h"

int main() {
  f<char>();
}

/**** IWYU_SUMMARY

(tests/bugs/1942/1942.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
