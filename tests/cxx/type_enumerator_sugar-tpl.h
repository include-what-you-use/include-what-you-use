//===--- type_enumerator_sugar-tpl.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

template <class A, class B>
struct Pair {
  using first = A;
  using second = B;
};

template <class T>
struct Holder {
  typename T::first first;
  typename T::second second;
};
