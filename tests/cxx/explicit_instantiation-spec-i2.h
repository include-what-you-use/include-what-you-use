//===--- explicit_instantiation-spec-i2.h - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/explicit_instantiation-template.h"

template <>
void TplFn<char>();

template <>
inline char var_tpl<float>;

template <typename T>
double var_tpl<T*>;
