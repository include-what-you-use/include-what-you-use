//===--- explicit_instantiation-template2.h - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

template <typename T>
class ClassWithUsingMethod2 {
 public:
  void Fn() {
    T t;
  }

  void OutOfLineMethod();
};

template <typename T>
void ClassWithUsingMethod2<T>::OutOfLineMethod() {
  T t;
}

template <typename T = IndirectClass>
class TplWithProvidedDefArg {
  T t;
};
