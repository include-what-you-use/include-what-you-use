//===--- 1651.h - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <string>

struct S {
  std::string str;
};

/**** IWYU_SUMMARY

(tests/bugs/1651/1651.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
