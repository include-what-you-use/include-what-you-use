//===--- cvr-derived.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_CVR_DERIVED_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_CVR_DERIVED_H_

#include "tests/cxx/cvr-base.h"

class Derived : public Base {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_CVR_DERIVED_H_
