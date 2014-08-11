//===--- badinc-i1.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I1_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I1_H_

#include <stddef.h>  // For offsetof (in badinc.cc).
#include <stdlib.h>  // also included from badinc-d1.h -- before this file
#include <stdio.h>   // also included from badinc-d3.h -- after this file
#include "tests/cxx/badinc-d2.h"
#include "tests/cxx/badinc-i2.h"
#if 0
#include "tests/cxx/badinc-i5.h"
#endif
#include "tests/cxx/badinc-i6.h"

#define MACRO_CALLING_I6_FUNCTION  I6_Function()
#define I1_MACRO_SYMBOL_WITHOUT_VALUE
#define I1_MACRO_SYMBOL_WITH_VALUE 1
#define I1_MACRO_SYMBOL_WITH_VALUE0 0
#define I1_MACRO_SYMBOL_WITH_VALUE2 2

// Lets us test handling of operator<< when first argument is a macro.
#define I1_MACRO_LOGGING_CLASS \
  I2_OperatorDefinedInI1Class logging_class = I2_OperatorDefinedInI1Class()

// The types.
enum I1_Enum { I11, I12, I13 };

const int kI1ConstInt = 5;

typedef I2_Typedef I1_Typedef;    // nested typedefs

struct I1_Struct {
  int a;
  float b;
  I1_Enum c;
};

union I1_Union {
  I1_Struct* a;
  I1_Enum b;
  int c;
};

typedef struct {
  int a;
  float b;
} I1_UnnamedStruct;

class I1_Base {
 public:
  virtual ~I1_Base() {}
};

class I1_Class : public I1_Base {
 public:
  explicit I1_Class(int a) { a_ = a; }
  I1_Class() { a_ = 1; }
  // Call with T == I1_NamespaceClass, say.
  template<typename T> I1_Class(const T* obj, int dummy) : a_(obj->a) { }
  struct NestedStruct;    // defined below
  template<class T> struct NestedTemplateStruct { };
  int a() const { return a_; }
  I1_Typedef b() const { return static_cast<I2_EnumForTypedefs>(a_); }
  template<typename A> I1_Enum I1_ClassTemplateFunction(A a) { return I11; }
  template<typename A> static I1_Enum I1_StaticClassTemplateFunction(A a) {
    return I12;
  }
  static int s() { return 1; }
  typedef int I1_Class_int;
 private:
  int a_;
  static I1_Class_int s_;
};
I1_Class::I1_Class_int I1_Class::s_;
struct I1_Class::NestedStruct {
  typedef int NestedStructTypedef;
};

class I1_SiblingClass : public I1_Base {
};

class I1_Subclass : public D2_Class {
};

template<typename FOO, typename BAR=FOO>
class I1_TemplateClass {
 public:
  I1_TemplateClass() { BAR bar; (void)bar; }
  I1_TemplateClass(FOO a) { a_ = a; }
  I1_TemplateClass(I1_Union a) { }
  template<typename CTOR> I1_TemplateClass(CTOR ctor_arg, bool unused) { }
  ~I1_TemplateClass() { FOO* tmp; tmp = &a_; BAR bar; (void)bar; }
  FOO a() { return a_; }
  void new_delete_bar() { BAR* bar = new BAR(); delete bar; }
  typedef int I1_TemplateClass_int;
 private:
  FOO a_;
  I1_TemplateClass<FOO>* next_;   // test type-recursion
};

template<typename FOO, typename BAR=FOO>
class I1_TemplateSubclass : public I1_TemplateClass<FOO,BAR> {
};

template<typename FOO, typename UNUSED = I1_Union>
class I1_TemplateMethodOnlyClass {
 public:
  FOO a() { FOO retval; return retval; }
  FOO* b() { FOO* retval = NULL; return retval; }
  static int stat() { FOO foo; (void)foo; return 2; }
  template<typename BAR> BAR c() { return BAR(); }
  template<typename BAR> BAR* d() { BAR* retval = NULL; return retval; }
  template<typename BAR> int e(BAR* b) { return (int)b->size(); }
  template<typename BAR> static int s(BAR b) {   // BAR should be a ptr type
    FOO foo;
    return foo.a() + (int)b->size();
  }
  template<typename BAR> static int t() {   // BAR should be I1_Class
    I1_TemplateClass<typename BAR::I1_Class_int> tc;
    return tc.a();
  }
  // BAR should be I1_TemplateClass
  template<template<typename T, typename U=T> class BAR> static int tt() {
    BAR<I1_Class> t;
    return t.a().a();
  }
};

template<typename FOO, typename BAR> class I1_TemplateClassFwdDeclaredInD2 {
};

template<typename FOO> class I1_TypedefOnly_Class {
 public:
  typedef typename FOO::I1_Class_int i;   // best if FOO == I1_Class
};

// This class is only referenced via D1_I1_Typedef, in badinc-d1.h
class I1_Typedef_Class {
 public:
  I1_Typedef_Class() { a_ = 1; }
  int a() { return a_; }
 private:
  int a_;
};

template<typename T> class I1_const_ptr {
 public:
  explicit I1_const_ptr(const T* ptr) : ptr_(ptr) { }
  ~I1_const_ptr() { delete ptr_; }
  const T& operator*() { return *ptr_; }
  const T* operator->() { return ptr_; }
  int operator~() { return deref_a(); }
  void del() { delete (((ptr_))); }
  void indirect_del() { del(); }
  int deref_a() { return (*ptr_)->a(); }
 private:
  const T* ptr_;
};
// Try an out-of-line operator too, both regular-style and yoda-style.
template<typename T> bool operator==(I1_const_ptr<T>& ptr, const T& val) {
  return &*ptr == &val;
}
template<typename T> bool operator==(const T& val, I1_const_ptr<T>& ptr) {
  return &*ptr == &val;
}

class I1_DefinedInCc_Class;

// This class is never used, but has an empty destructor defined in
// badinc.cc.  We want to make sure we don't get '(ptr only)' for it.
class EmptyDestructorClass {
 public:
  EmptyDestructorClass() { }
  ~EmptyDestructorClass();
};

// This is needed by Cc_TemplateClass.  OperateOn is from badinc.h
template<class T> class OperateOn;   // forward declare, from badinc.h
template<> class OperateOn<I1_Struct> { };
template<class T> class OperateOn<I1_TemplateClass<T> > { };

typedef I2_Class I1_I2_Class_Typedef;
typedef I1_Class* I1_ClassPtr;

typedef I1_Enum (*I1_FunctionPtr)(I1_Class*);

I1_Enum I1_Function(I1_Class* c) {
  return I11;
}

inline void I1_OverloadedFunction(int i) { }
inline void I1_OverloadedFunction(float f) { }
template<typename T> void I1_OverloadedFunction(const T& t) { }

inline void I1_And_I2_OverloadedFunction(int i) { }

template<typename A> I1_Enum I1_TemplateFunction(A a) {
  return I11;
}


I2_OperatorDefinedInI1Class& I2_OperatorDefinedInI1Class::operator<<(int i) {
  return *this;
}

class I1_ForwardDeclareClass;

struct I1_ManyPtrStruct {
  int a;
};

struct I1_PtrDereferenceStruct {
  int a;
};

class I1_PtrDereferenceClass {
 public:
  int a() { return 1; }
};

class I1_PtrDereferenceStatic {
 public:
  static int a() { return 1; }
};

class I1_StaticMethod {
 public:
  static int a() { return 1; }
};

class I1_MemberPtr {
 public:
  int a() { return 1; }
};

class I1_PtrAndUseOnSameLine {
 public:
  int a() { return 1; }
};

class I1_SubclassesI2Class : public I2_Class {
 public:
  int a() { return 1; }
};

namespace i1_ns {
class I1_NamespaceClass {
 public:
  int a;
};
struct I1_NamespaceStruct {
  int a;
};
struct I1_UnusedNamespaceStruct {
  int a;
};
template<typename T> void I1_NamespaceTemplateFn(T t) { }
}

// Defines a class that's declared in badinc.h.
class H_Class::H_Class_DefinedInI1 { };


// The vars.  Just a few.
int i1_int = 6;
I1_Enum i1_i1_enum;
I1_Union i1_i1_union;
I1_UnnamedStruct i1_i1_unnamed_struct;
I1_Class* i1_i1_classptr;
D2_Class i1_d2_class;
namespace i1_ns { int i1_int_global; }
namespace i1_ns { namespace i1_subns { int i1_int_globalsub; } }
namespace i1_ns2 { int i1_int_global2; }
namespace i1_ns2 { namespace i1_subns { int i1_int_global2sub; } }
namespace i1_ns3 { int i1_int_global3; }
namespace i1_ns3 { namespace i1_subns { int i1_int_global3sub; } }
namespace i1_ns4 { int i1_int_global4; }
namespace i1_ns4 { namespace i1_subns { int i1_int_global4sub; } }
namespace i1_ns5 { int i1_unused_global; }
int i1_GlobalFunction(void) { return 1; }

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I1_H_
