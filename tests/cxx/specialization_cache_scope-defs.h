//===--- specialization_cache_scope-defs.h - test input -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_provider-payload.h"

template <class L, class R>
struct Pair {};

// The qualifier makes the expansion use its argument fully.
template <class T>
using WithType = Pair<typename T::type, T>;

// The member type does not depend on T, so every Member specialization
// reaches the same expansion of WithType<Payload>.
template <class T>
struct Member {
  WithType<Payload> value;
};

template <class T>
struct Holder {
  Member<T> value;
  Member<T*> pointer;
};

template <class T>
struct Reuser {
  Member<T*> pointer;
};
