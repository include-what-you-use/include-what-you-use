//===--- type_trait-i2.h - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPE_TRAIT_I2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPE_TRAIT_I2_H_

#include "tests/cxx/type_trait-i1.h"

class Derived : public Base {};

struct With3WayComp {
  int operator<=>(int) const;
  int operator<=>(int**) const;
  int operator<=>(const Struct&) const;
  int operator<=>(const Base*) const;
  int operator<=>(const Union1*) const;
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPE_TRAIT_I2_H_
