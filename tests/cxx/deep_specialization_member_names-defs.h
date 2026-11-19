//===--- deep_specialization_member_names-defs.h - test input -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

struct Payload {
  int x;
};

template <class A, class B>
struct Pair {
  A a;
  B b;
};

template <class T>
using Dup = Pair<T, T>;

template <class T>
struct Outer {
  template <class U>
  struct Member {
    U u;
  };
  // Named like class templates and typedefs that IWYU looks up by name.
  template <class U>
  struct map {
    U u;
  };
  typedef int ostream;
};
