//===--- 958.h - iwyu test ------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename T>
int templFunc(T);

/**** IWYU_SUMMARY

(tests/bugs/958/958.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
