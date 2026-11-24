//===--- specialization_use_member_count-root.h - test input --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_member_count-defs.h"

// Uses its argument as a qualifier, which needs the full type.
template <class T>
using Q = ATQ<T, T::template Nested>;

template <class T>
using Twice = Sink<T, T>;

template <class T>
struct Holder {
  Twice<Twice<Twice<Q<Wrap<T>>>>> value;
};
