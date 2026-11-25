//===--- specialization_provider-root.h - test input ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_provider-defs.h"

template <template <class> class L, template <class> class R, class T>
using Both = Sink<L<T>, R<T>>;

template <template <class> class L, template <class> class R, class T>
struct Root {
  Both<L, R, Box<T>> value;
};

// Repeated descendants exercise reuse below another class traversal.
template <class T>
using Twice = Sink<T, T>;

template <template <class> class L, template <class> class R, class T>
struct RepeatedRoot {
  Twice<Both<L, R, Box<T>>> value;
};

template <template <class> class L, template <class> class R, class T>
struct Outer {
  RepeatedRoot<L, R, T> root;
};

template <template <class> class L, template <class> class R, class T>
void UseInBody() {
  Twice<Both<L, R, Box<T>>> local;
  (void)local;
}
