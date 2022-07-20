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
#include "tests/cxx/template_args-d1.h"
#include "tests/cxx/template_args-d2.h"

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
  static ProvidingAlias<5> aliasedProviding;
  static NonProvidingAlias<5> aliasedNonProviding;
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

  // Test obtaining template argument through alias template, typedef and
  // several other layers of sugar. Underlying type provision status of aliases
  // should be accounted for.

  // ProvidingAlias provides TplInI1 but doesn't provide IndirectClass.
  // IWYU: IndirectClass is...*indirect.h
  Outer<decltype(StaticTemplateFieldStruct::aliasedProviding)> oapi;
  // TODO(bolshakov): IWYU: IndirectClass is...*indirect.h
  (void)oapi.t;

  Outer<decltype(StaticTemplateFieldStruct::aliasedProviding)*> oapip;
  (void)oapip.t;

  Outer<decltype(StaticTemplateFieldStruct::aliasedProviding)>* opapi;
  // TODO(bolshakov): IWYU: IndirectClass is...*indirect.h
  (void)opapi->t;

  // IWYU: TplInI1 is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  Outer<decltype(StaticTemplateFieldStruct::aliasedNonProviding)> oanpi;
  // IWYU: TplInI1 is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)oanpi.t;

  Outer<decltype(StaticTemplateFieldStruct::aliasedNonProviding)*> oanpip;
  (void)oanpip.t;

  Outer<decltype(StaticTemplateFieldStruct::aliasedNonProviding)>* opanpi;
  // IWYU: TplInI1 is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)opanpi->t;
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

template <typename T>
class TemplateWithFwdDeclUse {
  T* t;
};

// IWYU should not suggest neither TplInI1 nor IndirectClass full info
// for fwd-decl use in TemplateWithFwdDeclUse.
// IWYU: TplInI1 needs a declaration
// IWYU: IndirectClass needs a declaration
TemplateWithFwdDeclUse<TplInI1<IndirectClass>> c;

// ---------------------------------------------------------------

/**** IWYU_SUMMARY

tests/cxx/template_args.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/template_args-i1.h"

tests/cxx/template_args.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/template_args.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/template_args-d1.h"  // for ProvidingAlias
#include "tests/cxx/template_args-d2.h"  // for NonProvidingAlias
#include "tests/cxx/template_args-i1.h"  // for TplInI1
template <typename F> struct FunctionStruct;  // lines XX-XX

***** IWYU_SUMMARY */
