//===--- macro_location_tpl-d1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/macro_location_tpl-i1.h"
#include "tests/cxx/macro_location_tpl-i2.h"

// For the test to work correctly, '-i3.h' should not be included here directly.

template <typename T>
void UseProvidedFn1Taking(T t) {
  // Fn1(Struct) should be considered as provided because '-i2.h' with its
  // macro-expanded definition is directly included here.
  Fn1(t);
}

template <typename T>
void CallFn2WithProvidingMacro(T t) {
  CALL_FN2(t);
}

#define DEFINE_CALL_FN3 \
  template <typename T> \
  void CallFn3(T t) {   \
    Fn3(t);             \
  }

DEFINE_USE_PROVIDED_DEF_ARG_3
