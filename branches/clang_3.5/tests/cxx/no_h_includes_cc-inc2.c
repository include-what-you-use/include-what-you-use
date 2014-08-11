//===--- no_h_includes_cc-inc2.c - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A .cc file that's included by another .cc file, and uses symbols
// defined in that other .cc file.  We should not suggest adding any
// #includes.

#if INCLUDED_FROM_MAIN
const int inc2 = 2;
#endif

/**** IWYU_SUMMARY

(tests/cxx/no_h_includes_cc-inc2.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
