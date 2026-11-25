//===--- macro_defined_by_includer_mapped-d1.h - test input file for iwyu -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_DEFINED_BY_INCLUDER_MAPPED_D1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_DEFINED_BY_INCLUDER_MAPPED_D1_H_

struct ExportedValue {
  int v;
};

#define EXPORTED_VALUE_CHAR_T char
#include "tests/cxx/macro_defined_by_includer_mapped-i1.h"  // IWYU pragma: export
#undef EXPORTED_VALUE_CHAR_T

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_DEFINED_BY_INCLUDER_MAPPED_D1_H_
