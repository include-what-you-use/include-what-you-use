//===--- specialization_use_order-root.h - test input ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_order-defs.h"

// Uses the explicit instantiation of the argument.
template <class T>
using UseInst = ATQ<T, T::template Nested>;

template <class X, class Y>
using XYX = Sink<X, Y, X>;

// Uses Box<Payload>, Crate<Payload>, and then Box<Payload> again (and
// Crate<Payload> in between) when the alias is expanded.
template <class T>
struct Holder {
  XYX<UseInst<Box<T>>, UseInst<Crate<T>>> value;
};
