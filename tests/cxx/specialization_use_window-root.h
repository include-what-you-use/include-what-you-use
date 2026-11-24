//===--- specialization_use_window-root.h - test input --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_window-defs.h"
#include "tests/cxx/specialization_use_window-payload.h"

// Uses the explicit instantiation of the argument.
template <class T>
using UseInst = ATQ<T, T::template Nested>;

// Uses the argument, then Crate<Payload>, then the argument again.
template <class X>
using XCX = Sink<X, UseInst<Crate<Payload>>, X>;

template <class T>
struct Holder {
  XCX<UseInst<Box<T>>> value;
};
