//===--- fn_def_args-i1.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/fn_def_args-i2.h"

void FnWithSmearedDefArgs2(int = 0, int);
void FnWithSmearedDefArgs3(int, int = 0, int);

template <typename T>
void FnTplWithDefArg(int) {
}
template <>
void FnTplWithDefArg<float>(int) {
}

template <typename T>
void* operator new(std::size_t, T, int) noexcept {
  return nullptr;
}
