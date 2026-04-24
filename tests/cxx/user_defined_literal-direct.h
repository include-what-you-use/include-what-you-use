//===--- user_defined_literal-direct.h - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file is meant to be directly included by the test .cc file.  It only
// re-exports the header that declares the literal operator.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_USER_DEFINED_LITERAL_DIRECT_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_USER_DEFINED_LITERAL_DIRECT_H_

#include "user_defined_literal-indirect.h"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_USER_DEFINED_LITERAL_DIRECT_H_
