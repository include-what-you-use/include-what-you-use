//===--- associated_impl_other.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also=tests/cxx/associated_impl_other-d1.inline -Xiwyu --mapping_file=tests/cxx/associated_impl_other.imp -I .

// Tests that an implementation file is analyzed together with its header
// when that header is not the main file's own, here because the
// implementation file is reached through '--check_also'.

#include "tests/cxx/associated_impl_other-d1.h"

void Use() {
  OtherInlineClass c;
}

/**** IWYU_SUMMARY

(tests/cxx/associated_impl_other.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
