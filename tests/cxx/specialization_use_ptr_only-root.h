//===--- specialization_use_ptr_only-root.h - test input ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

template <class... Ts>
struct Sink {};

template <class T, template <class> class N>
struct ATQ {};

// Uses its argument fully.
template <class T>
using UseFull = ATQ<T, T::template Nested>;

template <class X>
using Dup = Sink<X, X>;

// Uses the argument eight times when the alias is expanded.
template <class T>
struct Holder {
  Dup<Dup<Dup<UseFull<T>>>> value;
};
