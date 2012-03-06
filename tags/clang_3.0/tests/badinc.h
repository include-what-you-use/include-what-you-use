//===--- badinc.h - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_H_

#include <ctype.h>  // used only in badinc.cc
#include <errno.h>  // used both here and in badinc.cc
#include <math.h>
#include <queue>    // used only in this .h file, not in any other file.
#include <string>
#include "tests/badinc-d2.h"
#include "tests/badinc-d3.h"

class H_ForwardDeclareClass;

class Cc_Struct;   // test having the wrong 'kind' (Cc_Struct is a struct)
class Cc_Class;
enum H_Enum { H1, H2, H3 };

template<typename T> class I2_TypedefOnly_Class;
typedef I2_TypedefOnly_Class<int> H_I1_Class_Typedef;

// H_ScopedPtr and H_MakeScopedPtr mimic the implementation of
// scoped_ptr but are much simplified.

template <typename T> class H_ScopedPtr;
template <typename T> H_ScopedPtr<T> H_MakeScopedPtr(T *);

template <typename T>
class H_ScopedPtr {
 public:
  typedef T element_type;
  typedef T* element_ptr;
  typedef std::queue<T> element_queue;
  T* get() { return ptr_; }
  T& operator*() { return *ptr_; }
  T* operator->() { return ptr_; }
  ~H_ScopedPtr() {
    enum { type_must_be_complete = sizeof(T) };
    delete ptr_;
  }

 private:
  friend H_ScopedPtr<T> H_MakeScopedPtr<T>(T* p);

  T* ptr_;
};

template <typename T> H_ScopedPtr<T> H_MakeScopedPtr(T* p) {
  return H_ScopedPtr<T>(p);
}

// This is for the implicit constructor and implicit destructor:
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
class H_Pimpl {
 private:
  H_ScopedPtr<Cc_Class> cc_impl_;
  // IWYU: I2_Class needs a declaration
  H_ScopedPtr<I2_Class> i2_impl_;
};

// These are for the implicit destructor:
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
// IWYU: I2_Struct is...*badinc-i2.h
class H_Pimpl_ExplicitCtor {
 public:
  H_Pimpl_ExplicitCtor() : i2_impl_explicit_ctor_() { }
 private:
  H_ScopedPtr<Cc_Class> cc_impl_;
  // IWYU: I2_Class needs a declaration
  H_ScopedPtr<I2_Class> i2_impl_;
  // IWYU: I2_Struct needs a declaration
  H_ScopedPtr<I2_Struct> i2_impl_explicit_ctor_;
};

class H_Pimpl_ExplicitCtorDtor {
 public:
  H_Pimpl_ExplicitCtorDtor() : i2_impl_explicit_ctor_() { }
  ~H_Pimpl_ExplicitCtorDtor();
 private:
  H_ScopedPtr<Cc_Class> cc_impl_;
  // IWYU: I2_Class needs a declaration
  H_ScopedPtr<I2_Class> i2_impl_;
  // IWYU: I2_Struct needs a declaration
  H_ScopedPtr<I2_Struct> i2_impl_explicit_ctor_;
};

template <typename T> struct H_ScopedPtrHolder {
  H_ScopedPtr<T> holder;
};


class H_Class {
 public:
  class H_Class_Subdecl;
  class H_Class_UnusedSubdecl;   // defined in badinc.cc
  class H_Class_DefinedInI1;     // defined in badinc-i1.h
  H_Class(int a) { a_ = a; }
  // IWYU: I2_Enum is...*badinc-i2.h
  H_Class(I2_Enum i2_enum) { a_ = 1; }
  // IWYU: I2_MACRO is...*badinc-i2.h
  H_Class() { a_ = I2_MACRO; }    // from badinc-i2.h
  int a() { return a_; }
  H_Enum b() const { return static_cast<H_Enum>(a_); }
  // IWYU: I2_Typedef is...*badinc-i2.h
  // IWYU: I2_EnumForTypedefs is...*badinc-i2.h
  I2_Typedef c() const { return static_cast<I2_EnumForTypedefs>(a_); }
  // IWYU: I2_Struct is...*badinc-i2.h
  I2_Struct unused_c() const { return I2_Struct(); }
  // IWYU: I2_Struct is...*badinc-i2.h
  int unused_c2() const { return I2_Struct().a; }
  D3_Enum d() const { return static_cast<D3_Enum>(a_); }
  int e() const {
    std::string s("a long long string");
    std::queue<int> q;
    if (q.empty()) return s.length();
    // IWYU: I2_Enum is...*badinc-i2.h
    switch (static_cast<I2_Enum>(a_)) {
      // IWYU: I21 is...*badinc-i2.h
      case I21: return 21;
        // IWYU: I22 is...*badinc-i2.h
      case I22: return 22;
        // IWYU: I23 is...*badinc-i2.h
      case I23: return 23;
      default: return errno;
    }
  }
  // IWYU: I2_Enum is...*badinc-i2.h
  int f(I2_Enum i2_enum);
  int g(H_Class_Subdecl *h_class_subdecl) { return 1; }
  // IWYU: TemplateForHClassTplFn needs a declaration
  template<typename A> A TplFn(const TemplateForHClassTplFn<A>& a) {
    return a.value;
  }
  // IWYU: TemplateForHClassTplFn needs a declaration
  int NonTplFn(const TemplateForHClassTplFn<int>& a) {
    return 0;
  }
  // IWYU: I2_Enum is...*badinc-i2.h
  static int static_out_of_line(I2_Enum i2_enum);
  struct H_NestedStruct {
    // IWYU: I2_Enum is...*badinc-i2.h
    I2_Enum nested_i2_enum;
    // IWYU: I2_Enum is...*badinc-i2.h
    int nested(I2_Enum i2_enum);
    // IWYU: I2_Enum is...*badinc-i2.h
    static int static_nested(I2_Enum i2_enum);
  };
  void DefinedInBadincCc();
  void UsedInBadincH() { DefinedInBadincCc(); }
  H_NestedStruct h_nested_struct;
  ~H_Class() {
    // IWYU: printf is...*<stdio.h>
    printf("%d/%d/%d/%d/%d\n", b(), c(), d(), e(), ee_);
  }
  // IWYU: I2_Enum is...*badinc-i2.h
  static I2_Enum ee_;
  // IWYU: I2_Enum is...*badinc-i2.h
  static I2_Enum ff_;
 private:
  // IWYU: I2_EnumForTypedefs is...*badinc-i2.h
  typedef I2_EnumForTypedefs H_Class_I2_Typedef;
  // IWYU: I2_Struct needs a declaration
  // IWYU: I2_Class needs a declaration
  friend I2_Struct I2_Function(I2_Class*);
  friend class I2_Class;
  template<typename FOO> friend class TemplateForHClassTplFn;
  int a_;
  Cc_Struct* ptr_into_cc_file_type_;
  H_Class(const H_Class&);
};
// IWYU: I2_Enum is...*badinc-i2.h
// IWYU: I21 is...*badinc-i2.h
I2_Enum H_Class::ee_ = I21;

template<typename FOO>
class H_TemplateClass {
 public:
  H_TemplateClass(FOO a) { a_ = a; }
  FOO a() { return a_; }
  H_Enum b() const { return static_cast<H_Enum>(a_); }
  // IWYU: I2_Typedef is...*badinc-i2.h
  // IWYU: I2_EnumForTypedefs is...*badinc-i2.h
  I2_Typedef c() const { return static_cast<I2_EnumForTypedefs>(a_); }
  // IWYU: I2_Struct is...*badinc-i2.h
  I2_Struct unused_c() const { return I2_Struct(); }  // unused tpl fn not iwyu
  // IWYU: I2_Struct is...*badinc-i2.h
  int unused_c2() const { return I2_Struct().a; }
  int unused_c3() const { return this_is_ok_even_though_it_exists_not(FOO()); }
  // IWYU: I2_Enum is...*badinc-i2.h
  int f(I2_Enum i2_enum);
  void uses_i2class() {
    // IWYU: I2_Class is...*badinc-i2.h
    // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
    I2_Class i2_class;
    (void)i2_class;
  }
  // IWYU: I2_Enum is...*badinc-i2.h
  static FOO static_out_of_line(I2_Enum i2_enum);
  // IWYU: I2_Enum is...*badinc-i2.h
  static FOO static_never_defined(I2_Enum i2_enum);
  struct H_TplNestedStruct {
    // IWYU: I2_Enum is...*badinc-i2.h
    I2_Enum tplnested_i2_enum;
    // IWYU: I2_Enum is...*badinc-i2.h
    int tplnested(I2_Enum i2_enum);
    // IWYU: I2_Enum is...*badinc-i2.h
    static FOO static_tplnested(I2_Enum i2_enum);
  };
  H_TplNestedStruct h_nested_struct;
  // IWYU: I2_Enum is...*badinc-i2.h
  static I2_Enum h_template_i2_static_;
  static FOO h_template_foo_static_;
 private:
  // IWYU: I2_EnumForTypedefs is...*badinc-i2.h
  typedef I2_EnumForTypedefs H_TemplateClass_I2_Typedef;
  // IWYU: I2_Struct needs a declaration
  // IWYU: I2_Class needs a declaration
  friend I2_Struct I2_Function(I2_Class*);
  FOO a_;
 public:
  H_TemplateClass(const H_TemplateClass&);
};
// IWYU: I2_Enum is...*badinc-i2.h
// IWYU: I21 is...*badinc-i2.h
template<typename FOO> I2_Enum H_TemplateClass<FOO>::h_template_i2_static_ = I21;
H_TemplateClass<int>* h_templateclass_var;

// IWYU: I2_TemplateClass is...*badinc-i2.h
template<template<typename A> class T = I2_TemplateClass>
// This is from the default destructor destroying t.
// TODO(csilvers): attribute this use here, not at the caller sites.
// TODO(csilvers): IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
class H_TemplateTemplateClass {
 public:
  // TODO(csilvers): attribute this use here, not at the caller sites.
  // TODO(csilvers): IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
  // IWYU: I2_Enum is...*badinc-i2.h
  // IWYU: I2_Enum::I21 is...*badinc-i2.h
  H_TemplateTemplateClass() : t(T<I2_Enum>(I21)) {}
  // IWYU: I2_Enum is...*badinc-i2.h
  T<I2_Enum> t;
  // IWYU: I2_Enum is...*badinc-i2.h
  I2_Enum e;
};

// The generic OperateOn, but each specialization needs to define its own.
template<class T> class OperateOn { };

// OperateOn isn't checked for IWYU violations until it's instantiated.
template<class T, class Functor = OperateOn<T> > class H_TemplateStructHelper {
 public:
  void a() {
    Functor f;
    (void)f;
  }
};

// To make this example as much like hash_set<> as possible, the outer
// class is really just a container around the class that does work.
template<class T, class Functor = OperateOn<T> > class H_TemplateStruct {
 private:
  typedef H_TemplateStructHelper<T, Functor> _TS;
  _TS ts;
 public:
  void a() { return ts.a(); }
};

template<class T> struct H_TypedefStruct {
  // Should not be an iwyu violation for T
  typedef T t_type;
  // IWYU: std::pair is...*<utility>
  typedef std::pair<T, T> pair_type;
};

// IWYU: I2_EnumForTypedefs is...*badinc-i2.h
typedef I2_EnumForTypedefs H_Typedef;
// IWYU: std::set is...*<set>
// IWYU: I2_Enum is...*badinc-i2.h
typedef std::set<I2_Enum> H_I2Enum_Set;
// We need the full definition of I2_Class because as a typedef we are
// re-exporting the vector<I2_Class> type, so it must be fully defined.
// TODO(csilvers): IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
// IWYU: std::vector is...*<vector>
// IWYU: I2_Class needs a declaration
// IWYU: I2_Class is...*badinc-i2.h
typedef std::vector<I2_Class> H_I2Class_Vector_Unused;
// IWYU: I2_TemplateClass is...*badinc-i2.h
// IWYU: I2_TemplateClass::I2_TemplateClass<.*> is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::InlFileTemplateClassFn is...*badinc-i2-inl.h
// IWYU: I2_Enum is...*badinc-i2.h
typedef I2_TemplateClass<I2_Enum> H_TemplateTypedef;

// IWYU: I2_Struct needs a declaration
typedef I2_Struct* H_StructPtr;

// IWYU: I2_Class needs a declaration
typedef int (*H_FunctionPtr)(int, I2_Class*);

H_Enum H_Function(H_Class* c) {
  return H1;
}

// IWYU: I2_Class needs a declaration
// IWYU: I2_Enum is...*badinc-i2.h
I2_Enum H_Function_I(I2_Class*) {
  // IWYU: I21 is...*badinc-i2.h
  return I21;
}

// IWYU: I2_Enum is...*badinc-i2.h
// IWYU: I2_Class needs a declaration
I2_Enum H_Function_I2(I2_Class* c);

template<typename A> int H_TemplateFunction(A a) {
  typedef A value_type;   // Should not cause an iwyu violation
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::I2_Class is...*badinc-i2-inl.h
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  // IWYU: I2_Class::InlFileFn is...*badinc-i2-inl.h
  // IWYU: I2_Class::InlFileTemplateFn is...*badinc-i2-inl.h
  // IWYU: I2_Class::InlFileStaticFn is...*badinc-i2-inl.h
  typedef I2_Class i2_type;
  // IWYU: I2_Class needs a declaration
  I2_Class* i2_class;
  // IWYU: NULL is...*<stdio.h>
  i2_class = NULL;
  return a == A() ? 1 : 0;
}

// This macro is tricky because myclass_##classname involves a type
// that's defined in scratch space.  Make sure this doesn't result in
// an IWYU violation.  Nor should classname used *not* in a macro
// concatenation (as the return value of Init).
#define H_USE_CLASS(classname)                  \
  struct H_Use_##classname {                    \
    H_Use_##classname() { Init(); }             \
    classname* Init() { return 0; }             \
  };                                            \
  static H_Use_##classname myclass_##classname

#define H_CREATE_VAR(typ)   typ h_create_var

template<typename T> T& Identity(T& t) { return t; }
#define H_IDENTITY(x)  Identity(x)


namespace h_ns {
// IWYU: I2_Struct is...*badinc-i2.h
typedef I2_Struct H_NamespaceTypedef;
}

// The vars.  Just a few.
H_Enum h_h_enum;
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
I2_Class h_i2_class;
H_TemplateClass<D3_Enum> h_d3_template_class(D31);
// IWYU: I2_Enum is...*badinc-i2.h
// IWYU: I22 is...*badinc-i2.h
H_TemplateClass<I2_Enum> h_i2_template_class(I22);
// TODO(csilvers): this should be attributed to the .h, since it comes
// via a default template argument.
// IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
H_TemplateTemplateClass<> h_templatetemlpate_class;
H_TemplateTemplateClass<H_TemplateClass> h_i2_templatetemlpate_class;

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_H_


/**** IWYU_SUMMARY

tests/badinc.h should add these lines:
#include <stdio.h>
#include <set>
#include <utility>
#include <vector>
#include "tests/badinc-i2-inl.h"
#include "tests/badinc-i2.h"

tests/badinc.h should remove these lines:
- #include <ctype.h>  // lines XX-XX
- #include <math.h>  // lines XX-XX
- #include "tests/badinc-d2.h"  // lines XX-XX
- class H_ForwardDeclareClass;  // lines XX-XX
- template <typename T> class I2_TypedefOnly_Class;  // lines XX-XX

The full include-list for tests/badinc.h:
#include <errno.h>  // for errno
#include <stdio.h>  // for NULL, printf
#include <queue>  // for queue
#include <set>  // for set
#include <string>  // for string
#include <utility>  // for pair
#include <vector>  // for vector
#include "tests/badinc-d3.h"  // for D3_Enum, D3_Enum::D31
#include "tests/badinc-i2-inl.h"  // for I2_Class::I2_Class, I2_Class::InlFileFn, I2_Class::InlFileStaticFn, I2_Class::InlFileTemplateFn, I2_Class::~I2_Class, I2_TemplateClass::I2_TemplateClass<FOO>, I2_TemplateClass::InlFileTemplateClassFn, I2_TemplateClass::~I2_TemplateClass<FOO>
#include "tests/badinc-i2.h"  // for I2_Class, I2_Enum, I2_Enum::I21, I2_Enum::I22, I2_Enum::I23, I2_EnumForTypedefs, I2_MACRO, I2_Struct, I2_TemplateClass, I2_Typedef, I2_TypedefOnly_Class (ptr only), TemplateForHClassTplFn (ptr only)
class Cc_Class;  // lines XX-XX
// TODO(csilvers): this should change to struct Cc_Struct.
class Cc_Struct;  // lines XX-XX
class H_Class::H_Class_DefinedInI1;  // lines XX-XX
class H_Class::H_Class_Subdecl;  // lines XX-XX
class H_Class::H_Class_UnusedSubdecl;  // lines XX-XX
template <typename T> class H_ScopedPtr;  // lines XX-XX

***** IWYU_SUMMARY */
