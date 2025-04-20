//===--- type_trait-d1.h - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPE_TRAIT_D1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPE_TRAIT_D1_H_

#include "tests/cxx/type_trait-i2.h"

using DerivedPtrRefProviding = Derived*&;
using DerivedRefProviding = Derived&;
using ClassRefProviding = Class&;
using ClassConstRefProviding = const Class&;
using Union1RefProviding = Union1&;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPE_TRAIT_D1_H_
