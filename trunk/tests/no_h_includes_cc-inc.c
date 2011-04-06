//===--- no_h_includes_cc-inc.c - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A .cc file that's included by another .cc file.

#define CC_INC_HAS_INT 1

const int kCcIncInt = 100;

/**** IWYU_SUMMARY

(tests/no_h_includes_cc-inc.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
