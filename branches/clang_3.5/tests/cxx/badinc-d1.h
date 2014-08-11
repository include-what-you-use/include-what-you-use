//===--- badinc-d1.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D1_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D1_H_

#include <stdlib.h>
#include <errno.h>    // not used, but iwyu shouldn't warn about that
// i1.h should come first, to make sure d3.h is seen after i1.h
#include "tests/cxx/badinc-i1.h"
#include "tests/cxx/badinc-d3.h"
#include "tests/cxx/badinc-i4.h"

// This uses a macro in an indirect-included file, but shouldn't be a
// violation, since we hard-coded this as an exception in iwyu_macromap.js.
#define MACRO_CALLING_I4_FUNCTION  I4_Function()

#define UNUSED_MACRO  Unknown_Function()

// The types.
enum D1_Enum { D11, D12, D13 };

class D1_Class {
 public:
  D1_Class(int a) { a_ = a; }
  D1_Class() { a_ = 1; }
  int a() { return a_; }
  D1_Enum b() const { return static_cast<D1_Enum>(a_); }
  I1_Typedef c() const { return static_cast<I2_EnumForTypedefs>(a_); }
  I1_Struct unused_c() const { return I1_Struct(); }
  int d() { return rand(); }
  ~D1_Class() {
    printf("%d/%d/%d\n", b(), c(), e_);
  }
  D1_Class& operator=(const D1_Class& that) {
    this->a_ = that.a_;
    return *this;
  }
  bool operator==(const D1_Class& that) const {
    return this->a_ == that.a_;
  }

  static I1_Enum e_;
  static I1_Enum f_;
 private:
  int a_;
};
I1_Enum D1_Class::e_ = I11;
I1_Enum D1_Class::f_ = I12;

class D1_Subclass : public I1_Class {
};

template<typename FOO>
class D1_TemplateClass {
 public:
  D1_TemplateClass(FOO a) { a_ = a; }
  FOO a() { return a_; }
 private:
  FOO a_;
};

class D1_CopyClass {
 public:
  D1_CopyClass(int a) { a_ = a; }
  int a() const { return a_; }
  D1_Class d1;
  int a_;
  // This class uses default copy constructor and operator=
};

template<typename FOO, typename BAR=I1_Enum>
struct D1_TemplateStructWithDefaultParam {
  FOO a;
  BAR b;
};

typedef I1_Typedef_Class D1_I1_Typedef;

typedef D1_Class* D1_StructPtr;

typedef int (*D1_FunctionPtr)(int, D1_Enum);

D1_Enum D1_Function(D1_Class* c) {
  return D11;
}

class D1_ForwardDeclareClass;

int D1Function(I1_Enum i=I11) {
  return static_cast<int>(i);
}

D1_CopyClass D1CopyClassFn(I1_Enum i) {
  return D1_CopyClass(static_cast<int>(i));
}

// The vars.  Just a few.
D1_Enum d1_d1_enum;
I1_Class d1_i1_class;

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D1_H_
