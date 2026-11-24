//===--- memo_provider_replay-root.h - test input -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/memo_provider_replay-defs.h"

template <class T>
using Twice = Sink<T, T>;

// Reaches the Box<T> qualifier through L, then through R.
template <template <class> class L, template <class> class R, class T>
using Both = Sink<L<T>, R<T>>;

template <template <class> class L, template <class> class R, class T>
struct Root {
  Twice<Both<L, R, Box<T>>> value;
};

// The same, from a class traversed while traversing another one.
template <template <class> class L, template <class> class R, class T>
struct Outer {
  Root<L, R, T> root;
};

// The same, from a function body.
template <template <class> class L, template <class> class R, class T>
void UseInBody() {
  Twice<Both<L, R, Box<T>>> local;
  (void)local;
}
