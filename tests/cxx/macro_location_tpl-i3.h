//===--- macro_location_tpl-i3.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define DEFINE_FN1_TAKING(Type) \
  inline void Fn1(Type) {       \
  }

#define DEFINE_TPL_FN_AND_FN2 \
  template <typename T>       \
  void Fn2() {                \
  }                           \
  template <typename T>       \
  void TplFn() {              \
    Fn2<T>();                 \
  }

#define DEFINE_USE_PROVIDED_DEF_ARG_2 \
  class DefArg2 {};                   \
  template <typename T = DefArg2>     \
  void UseProvidedDefArg2() {         \
    T t;                              \
  }

class DefArg3 {};

#define DEFINE_USE_PROVIDED_DEF_ARG_3 \
  template <typename T = DefArg3>     \
  void UseProvidedDefArg3() {         \
    T t;                              \
  }
