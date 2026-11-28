//===--- specialization_provider-defs.h - test input ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

template <class...> struct Sink {};

template <class T> struct Box {
  T value;
  template <class> struct Nested {};
};

template <class T, template <class> class N>
struct ArgumentThenQualifier {};

// Spelling and expansion locations must not confuse which header provides.
#define DEFINE_QUALIFIER(name) \
  template <class T>           \
  using name = ArgumentThenQualifier<T, T::template Nested>;

template <template <class> class L, template <class> class R, class T>
using Both = Sink<L<T>, R<T>>;

template <template <class> class L, template <class> class R, class T>
struct Root {
  Both<L, R, Box<T>> value;
};

// Repeated aliases exercise reuse through multiple paths to the same type.
template <class T>
using Twice = Sink<T, T>;

template <template <class> class L, template <class> class R, class T>
struct RepeatedRoot {
  Twice<Both<L, R, Box<T>>> value;
};

template <template <class> class L, template <class> class R, class T>
void UseInBody() {
  Twice<Both<L, R, Box<T>>> local;
  (void)local;
}
