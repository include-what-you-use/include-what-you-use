//===--- tpl_spill_nested-d2.h - test input file for iwyu ------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/tpl_spill_nested-d3.h"

template <class T>
struct Inner {
  typedef T value_type;
  typedef value_type* iterator;
  iterator begin();
  iterator end();

  Inner()
      // Use an internal type.
      : size(sizeof(detail::Detail)) {
    // Throw in a few statements that exercise type reporting.
    try {
    } catch (const Inner<T>&) {
    }

    for (const value_type& x : *this) {
    }
  }

  unsigned long long size;
};
