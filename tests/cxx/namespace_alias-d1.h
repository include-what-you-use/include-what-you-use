//===--- namespace_alias-d1.h - test input file for IWYU ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_NAMESPACE_ALIAS_D1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_NAMESPACE_ALIAS_D1_H_

// Nothing defined here, just including namespace_alias-i2.h and
// namespace_alias-i3.h - IWYU should recommend dropping this header

#include "tests/cxx/namespace_alias-i2.h"
#include "tests/cxx/namespace_alias-i3.h"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_NAMESPACE_ALIAS_D1_H_
