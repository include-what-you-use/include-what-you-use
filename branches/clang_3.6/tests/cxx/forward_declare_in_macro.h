//===--- forward_declare_in_macro.h - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Important that this macro be on line 11 of this file.
#define FORWARD_DECLARE_CLASS(cls)   class cls

/**** IWYU_SUMMARY

(tests/cxx/forward_declare_in_macro.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
