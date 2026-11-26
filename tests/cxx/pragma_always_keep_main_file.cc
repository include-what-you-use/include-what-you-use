//===--- pragma_always_keep_main_file.cc - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU pragma: always_keep

// Tests that an always_keep pragma in the main file doesn't crash IWYU.
// The main file is #included by <built-in>, without a spelling.

int x;

/**** IWYU_SUMMARY

(tests/cxx/pragma_always_keep_main_file.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
