//===--- fn_def_args-i2.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_FN_DEF_ARGS_I2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_FN_DEF_ARGS_I2_H_

#include <cstddef>

int GetInt();

void FnWithSmearedDefArgs(int, int = 0);
void FnWithSmearedDefArgs2(int, int = 0);
void FnWithSmearedDefArgs3(int, int, int);
void FnWithSmearedDefArgs3(int, int, int = 0);
void FnWithDefArg(int = 0);

void* operator new(std::size_t, int, int, int = 0);

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_FN_DEF_ARGS_I2_H_
