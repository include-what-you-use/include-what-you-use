//===--- badinc-d2.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D2_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D2_H_

// Forward-declare classes from badinc-i1.h before #including them.
// This tests that we find the real class definition, not the
// forward-declare just because it happens to come first.
namespace i1_ns { struct I1_NamespaceStruct; }
template<typename FOO, typename BAR> class I1_TemplateClassFwdDeclaredInD2;

#include "tests/badinc-i2-inl.h"

// Everything defined in this file is only forward-declared in badinc.cc.

enum D2_Enum { D21, D22, D23 };

class D2_Class {
 public:
  D2_Class(int a) { a_ = a; }
  D2_Class() { a_ = 1; }
  int a() { return a_; }
  D2_Enum b() const { return static_cast<D2_Enum>(a_); }
  I2_Class* c() const { return NULL; }
 private:
  int a_;
};

class D2_Subclass : public D2_Class {
};

template<typename FOO>
class D2_TemplateClass {
 public:
  D2_TemplateClass(FOO a) { a_ = a; }
  FOO a() { return a_; }
 private:
  FOO a_;
};

typedef D2_Class* D2_StructPtr;

typedef int (*D2_FunctionPtr)(int, D2_Enum);

// This shouldn't give an error because badinc-d2 isn't in the main CU.
I2_Enum D2_Function(D2_Class* c) {
  return I21;
}

class D2_ForwardDeclareClass;

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D2_H_
