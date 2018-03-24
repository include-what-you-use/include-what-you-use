//===--- macro_use_expanded_decl-macro.h - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_USE_EXPANDED_DECL_MACRO_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_USE_EXPANDED_DECL_MACRO_H_

#define GEN_METHOD() void gen_method();
#define CALL_METHOD(a) a.gen_method();
#define GEN_CLASS2() \
  struct Class2 {    \
    GEN_METHOD();    \
  };

#endif  //INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_USE_EXPANDED_DECL_MACRO_H_
