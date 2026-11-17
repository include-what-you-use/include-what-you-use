//===--- associated_pragma_order-d1.h - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/associated_pragma_order-i1.h"  // IWYU pragma: associated

// Uses IndirectClass without #including indirect.h: -i1.h does.
inline int UseD1() {
  IndirectClass local;
  return local.a + UseI1();
}

/**** IWYU_SUMMARY

(tests/cxx/associated_pragma_order-d1.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
