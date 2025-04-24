//===--- 1569.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

// The export pragma should protect this #include.
#include "f1.h"    // IWYU pragma: export

/**** IWYU_SUMMARY

(tests/bugs/1569/1569.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
