//===--- 297.cc - test input file for iwyu --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "t.h"
#include "o.h"

class Class : public Template<Object> {
  // If the function is called - it is tracked correctly
  // void bar() {
  //     Template<Object>::foo();
  // }

  // But when the function is made visible with using - it is not
  using Template<Object>::foo;
};

/**** IWYU_SUMMARY

(tests/bugs/297/297.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
