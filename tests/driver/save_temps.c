//===--- save_temps.c - test input file for iwyu --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Run IWYU with some variations of '-save-temps' to prove that it can still run
// to completion (see issue #1060).

// IWYU_ARGS: -save-temps --save-temps -save-temps=obj

struct Unused {};

/**** IWYU_SUMMARY

(tests/driver/save_temps.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
