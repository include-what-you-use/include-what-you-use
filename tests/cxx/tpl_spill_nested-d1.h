//===--- tpl_spill_nested-d1.h - test input file for iwyu ------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/tpl_spill_nested-d2.h"

template <class T>
struct Outer {
  Inner<T> inner;

  typename Inner<T>::value_type& operator[](int index) {
    return *(inner.begin() + index);
  }
};
