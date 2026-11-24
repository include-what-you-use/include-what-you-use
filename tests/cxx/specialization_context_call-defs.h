//===--- specialization_context_call-defs.h - test input ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_context_call-callee.h"

template <class...>
struct Sink {};

template <class T>
T& Ref();

// Converting to T needs T's full type, unless that happens in a call whose
// callee provides T.
template <class T, class S>
using Converted = Sink<decltype(static_cast<T>(Ref<S>()))>;

// Reaches U as an argument of a call, then outside of any call.
template <class U>
using CallThenPlain = Sink<decltype(Callee(U{}, Ref<Src>())), U>;

template <class T, class S>
struct Holder {
  CallThenPlain<Converted<T, S>> value;
};
