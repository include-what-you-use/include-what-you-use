//===--- 432.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include <stri\
ng>

/**** IWYU_SUMMARY

tests/bugs/432/432.cc should add these lines:

tests/bugs/432/432.cc should remove these lines:
- #include <string>  // lines XX-XX

The full include-list for tests/bugs/432/432.cc:

***** IWYU_SUMMARY */
