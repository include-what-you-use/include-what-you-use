//===--- re_fwd_decl.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class DeclaredInH;

DeclaredInH* use = 0;

/**** IWYU_SUMMARY

(tests/cxx/re_fwd_decl.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
