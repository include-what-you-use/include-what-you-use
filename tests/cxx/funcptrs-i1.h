//===--- funcptrs-i1.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_FUNCPTRS_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_FUNCPTRS_I1_H_

// Supporting types.
enum Enum { E_one };

class Retval {
 public:
  Retval() : value(0) {}
  Retval(int v) : value(v) {}

  int Value() const { return value; }

 private:
  int value;
};

// Functions of various kinds to which function pointers are bound.
class Class {
 public:
  int MemberFunction() const { return 20; }

  template<typename R>
  int MemberTemplate() const { return R(50).Value(); }

  static int StaticMemberFunction() { return 100; }

  template<typename R>
  static int StaticMemberTemplate() {
    return R(100).Value();
  }
};

Enum Function(Class*) {
  return E_one;
}

template<typename T>
int FunctionTemplate(Class*) {
  return T(10).Value();
}

template<typename T>
class ClassTemplate {
 public:
  int MemberFunction() const { return 20; }

  template<typename R>
  int MemberTemplate() const { return R(50).Value(); }

  static int StaticMemberFunction() {
    return 100;
  }

  template<typename R>
  static int StaticMemberTemplate() {
    return R(100).Value();
  }
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_FUNCPTRS_I1_H_
