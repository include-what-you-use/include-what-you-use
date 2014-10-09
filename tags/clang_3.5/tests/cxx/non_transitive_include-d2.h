//===--- non_transitive_include-d2.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// NOTE: this uses NonTransitiveIncludeD1, from d1.h, but does not
// #include d1.h.  But iwyu will not complain because we are in
// --transitive_includes_only mode.

NonTransitiveIncludeD1 nti_d2;


/**** IWYU_SUMMARY

(tests/cxx/non_transitive_include-d2.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
