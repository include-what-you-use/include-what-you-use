//===--- template_args.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests unusual and complex use of template arguments, such as
// function-proto template arguments, for both classes and functions.

#include "tests/cxx/direct.h"

// ---------------------------------------------------------------

// IWYU: IndirectClass needs a declaration
char Foo(IndirectClass ic);

template <typename F> struct FunctionStruct;
template <typename R, typename A1> struct FunctionStruct<R(A1)> {
  R result;
  char Argument1[sizeof(A1)];
};

void FunctionProtoClassArguments() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  FunctionStruct<char(IndirectClass&)> f1;
  (void)f1;

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  FunctionStruct<IndirectClass(char)> f2;
  (void)f2;
}

// ---------------------------------------------------------------

template <typename T> struct PointerStruct {   // T should be an IndirectClass*
  static int a(T t) { return t->a; }
};

template <typename T> struct DoublePointerStruct {
  static int a(T t) { return (*t)->a; }
};

template <typename T> struct PointerStruct2 {
  static int a(T* t) { return t->a; }
};

template <typename T> struct DoublePointerStruct2 {
  static int a(T** t) { return (*t)->a; }
};

void PointerClassArguments() {
  // IWYU: IndirectClass needs a declaration
  IndirectClass* ic = 0;

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  PointerStruct<IndirectClass*>::a(ic);

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  DoublePointerStruct<IndirectClass**>::a(&ic);

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  PointerStruct2<IndirectClass>::a(ic);

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  DoublePointerStruct2<IndirectClass>::a(&ic);
}

// ---------------------------------------------------------------

template<typename T> struct Outer { T t; };
template<typename T> struct Inner { T t; };
struct StaticTemplateFieldStruct {
  // IWYU: IndirectClass needs a declaration
  static Inner<IndirectClass> tpl;
};

void NestedTemplateArguments() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  Outer<Inner<IndirectClass> > oi;
  (void)oi;

  // IWYU: IndirectClass needs a declaration
  Outer<Inner<IndirectClass>* > oip;
  (void)oip;

  // IWYU: IndirectClass needs a declaration
  Outer<Inner<IndirectClass> >* opi;
  (void)opi;

  // Test that use of template specialization type template argument is not
  // hidden by any sugar in the AST.

  // IWYU: IndirectClass is...*indirect.h
  Outer<decltype(StaticTemplateFieldStruct::tpl)> osi;
  // Member referencing also requires template instantiation and nested member
  // full-type-use reporting.
  // IWYU: IndirectClass is...*indirect.h
  (void)osi.t;

  Outer<decltype(StaticTemplateFieldStruct::tpl)*> osip;
  (void)osip.t;

  Outer<decltype(StaticTemplateFieldStruct::tpl)>* opsi;
  // IWYU: IndirectClass is...*indirect.h
  (void)opsi->t;
}

// ---------------------------------------------------------------

template<typename T> struct CreateTemporary {
  static T a() { return T(); }
  template<typename U> static U b() { return U(); }
  static T statica() { return T(); }
  template<typename U> static U staticb() { return U(); }
};

void TestCreateTemporary() {
  // IWYU: IndirectClass needs a declaration
  CreateTemporary<IndirectClass> ct;

  // IWYU: IndirectClass is...*indirect.h
  ct.a();

  // IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectClass needs a declaration
  ct.b<IndirectClass>();

  // IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectClass needs a declaration
  CreateTemporary<IndirectClass>::statica();

  // IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectClass needs a declaration
  CreateTemporary<int>::staticb<IndirectClass>();
}

// ---------------------------------------------------------------

// IWYU: IndirectClass is...*indirect.h
typedef IndirectClass LocalClass;

void TestResugaringOfTypedefs() {
  FunctionStruct<char(LocalClass&)> f1;
  (void)f1;

  LocalClass* lc = 0;
  PointerStruct<LocalClass*>::a(lc);
  DoublePointerStruct<LocalClass**>::a(&lc);
  PointerStruct2<LocalClass>::a(lc);
  DoublePointerStruct2<LocalClass>::a(&lc);

  Outer<Inner<LocalClass> > oi;
  (void)oi;

  CreateTemporary<LocalClass>::a();
  CreateTemporary<int>::b<LocalClass>();
}

// ---------------------------------------------------------------


/**** IWYU_SUMMARY

tests/cxx/template_args.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/template_args.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/template_args.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
template <typename F> struct FunctionStruct;  // lines XX-XX

***** IWYU_SUMMARY */
