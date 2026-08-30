//===--- associated_impl_user.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/associated_impl_user.imp -I .

// Tests that a file using what an implementation file defines is told to
// #include its header, never the implementation file itself.

#include "tests/cxx/associated_impl_user-d1.h"

void Use() {
  UserInlineClass c;
  c.InlinedMethod();
}

/**** IWYU_SUMMARY

(tests/cxx/associated_impl_user.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
