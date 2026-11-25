//===--- provided_type_components_shared-aliases.h - test input -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef PROVIDED_TYPE_COMPONENTS_SHARED_ALIASES_H_
#define PROVIDED_TYPE_COMPONENTS_SHARED_ALIASES_H_

#include "tests/cxx/provided_type_components_shared-payload.h"

template <class A, class B>
struct Pair {
  A first;
  B second;
};

template <class T>
using Duplicate = Pair<T, T>;

using ProvidedPayload = Duplicate<Duplicate<Duplicate<Payload>>>;

#endif  // PROVIDED_TYPE_COMPONENTS_SHARED_ALIASES_H_
