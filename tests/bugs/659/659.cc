//===--- 659.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "macro_template.h"
#include "type.h"

void hello() {
  MACRO().func<Test>();    // If you write MacroClass().func<Test>();, it is OK
}

/**** IWYU_SUMMARY

(tests/bugs/659/659.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
