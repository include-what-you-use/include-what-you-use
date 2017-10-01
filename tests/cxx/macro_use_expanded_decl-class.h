//===--- macro_use_expanded_decl-class.h - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_USE_EXPANDED_DECL_CLASS_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_USE_EXPANDED_DECL_CLASS_H_

#include "macro_use_expanded_decl-macro.h"

struct Class {
  GEN_METHOD();
};

GEN_CLASS2();

#endif  //INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_USE_EXPANDED_DECL_CLASS_H_
