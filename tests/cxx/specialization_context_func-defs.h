//===--- specialization_context_func-defs.h - test input ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

enum class Color : int;

template <class...>
struct Sink {};

// Reaches U, then from the body of a lambda, which is another function.
template <class U>
using PlainThenLambda = Sink<U, decltype([] {
                               U u;
                               (void)u;
                             })>;

struct S {
  // __PRETTY_FUNCTION__ shows the enum template argument, so it needs Color
  // where it names this function, but not where it names the lambda.
  template <Color C>
  void F() {
    PlainThenLambda<Sink<decltype(__PRETTY_FUNCTION__)>> x;
    (void)x;
  }
};
