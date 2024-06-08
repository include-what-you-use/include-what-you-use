//===--- explicit_instantiation-template.h - test input file for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION_TEMPLATE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION_TEMPLATE_H_

#include "tests/cxx/direct.h"

template <typename T>
class Inner {
  T t;
};

template <typename T>
class Template {
  Inner<T> x;
};

template <typename T>
class ClassWithUsingMethod {
  void Fn() {
    T t;
  }
};

template <typename T>
class ClassWithMethodUsingPtr {
  void Fn() {
    T* t;
  }
};

template <typename, typename = void>
class TplWithDefArg {};

template <typename T>
class TplWithDefArg<int, T> {};

constexpr int getInt() {
  return 1;
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION_TEMPLATE_H_

/**** IWYU_SUMMARY

tests/cxx/explicit_instantiation-template.h should add these lines:

tests/cxx/explicit_instantiation-template.h should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/explicit_instantiation-template.h:

***** IWYU_SUMMARY */
