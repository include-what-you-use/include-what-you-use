//===--- no_h_includes_cc.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Uses a macro symbol and a non-macro symbol that are defined in the
// -inc.cc file.

#ifndef CC_INC_HAS_INT
const int x = kCcIncInt + 2;
#endif

/**** IWYU_SUMMARY

(tests/no_h_includes_cc.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
