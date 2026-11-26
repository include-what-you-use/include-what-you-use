//===--- implementation_file-d2.inl - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU pragma: private, include "tests/cxx/implementation_file-d2.h"

// Uses IndirectClass without #including indirect.h:
// implementation_file-d2.h does.

inline int ImplFileD2Class::InlineMethod() {
  IndirectClass local;
  return local.a;
}

/**** IWYU_SUMMARY

(tests/cxx/implementation_file-d2.inl has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
