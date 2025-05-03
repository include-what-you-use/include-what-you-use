//===--- 1685.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include "buffer.h"

struct Foo {
  Buffer<float> x;
};

/**** IWYU_SUMMARY

(tests/bugs/1685/1685.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
