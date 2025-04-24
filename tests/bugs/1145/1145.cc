//===--- 1145.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "subclass.h"

template <typename T>
struct TObjectPtr2 {
 public:
  template <typename U>
  TObjectPtr2(U&& Object) : ObjectPtr(Object) {
  }
  T* ObjectPtr;
};

struct TestClass {
 public:
  TObjectPtr2<BaseClass> Foo;
  TestClass(SubClass* InFoo) : Foo(InFoo) {
  }
};

/**** IWYU_SUMMARY

(tests/bugs/1145/1145.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
