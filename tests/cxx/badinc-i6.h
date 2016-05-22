//===--- badinc-i6.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Like badinc-i4, this file includes only routines that are used via
// macros from other files.  It's used to test a second codepath for
// the macro handling code (the "some more macro handling" in iwyu.js).

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_I6_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_I6_H_

int I6_Function() { return 1; }

#endif // INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_I6_H_
