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

// IWYU: IndirectClass is...*indirect.h
using IndirectClassProviding = IndirectClass;

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

  // IWYU: IndirectClass is...*indirect.h
  using ProvidingFunctionAlias = IndirectClass(IndirectClass);
  FunctionStruct<ProvidingFunctionAlias> f3;
  // IWYU: IndirectClass is...*indirect.h
  FunctionStruct<NonProvidingFunctionAlias1> f4;
  // IWYU: IndirectClass is...*indirect.h
  FunctionStruct<NonProvidingFunctionAlias2> f5;
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

template <typename T>
void FnUsingNested() {
  (void)sizeof(Outer<Inner<T>>);
}

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

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  FnUsingNested<IndirectClass>();

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
  // IWYU: IndirectClass is...*indirect.h
  (void)oapi.t;

  Outer<decltype(StaticTemplateFieldStruct::aliasedProviding)*> oapip;
  (void)oapip.t;

  Outer<decltype(StaticTemplateFieldStruct::aliasedProviding)>* opapi;
  // IWYU: IndirectClass is...*indirect.h
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

template <typename T>
T TplParamRetType() {
  throw 1;
}

// IWYU: IndirectClass needs a declaration
IndirectClass& i = TplParamRetType<IndirectClass&>();

template <typename T>
T TplParamRetTypeNoDef();

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
auto&& i2 = TplParamRetTypeNoDef<IndirectClass>();
auto&& i3 = TplParamRetTypeNoDef<IndirectClassProviding>();
// IWYU: IndirectClass is...*indirect.h
auto&& i4 = TplParamRetTypeNoDef<IndirectClassNonProviding>();

template <typename T>
struct TplWithMethodWithoutDef {
  static T GetT();
};

// IWYU: IndirectClass is...*indirect.h
using TplWithMethodWithoutDefProviding = TplWithMethodWithoutDef<IndirectClass>;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
auto&& i5 = TplWithMethodWithoutDef<IndirectClass>::GetT();
auto&& i6 = TplWithMethodWithoutDef<IndirectClassProviding>::GetT();
// IWYU: IndirectClass is...*indirect.h
auto&& i7 = TplWithMethodWithoutDef<IndirectClassNonProviding>::GetT();
auto&& i8 = TplWithMethodWithoutDefProviding::GetT();
// IWYU: IndirectClass is...*indirect.h
auto&& i9 = TplWithMethodWithoutDefNonProviding::GetT();

// ---------------------------------------------------------------

template <typename...>
void TplFnWithParameterPack() {
}

template <typename... Args>
void TplFnWithParameterPackUsingArgs1(Args&&... args) {
  // Capturing by copy requires complete type.
  [args...] {}();
}

template <typename... Args, typename T>
void TplFnWithParameterPackUsingArgs2(T t, Args&&... args) {
  [t, args...] {}();
}

template <typename... Args>
void TplFnWithParameterPackNonUsingArgs(Args&&... args) {
  [&args...] {}();
}

// IWYU: IndirectClass needs a declaration
void TestParameterPack(IndirectClass& ic,
                       // IWYU: IndirectTemplate needs a declaration
                       IndirectTemplate<int>& it,
                       // IWYU: TplInI1 needs a declaration
                       TplInI1<int>&& ti1) {
  // Test that IWYU doesn't crash on function template type parameter packs.
  TplFnWithParameterPack();
  // IWYU: IndirectClass is...*indirect.h
  TplFnWithParameterPackUsingArgs1(ic);
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectTemplate is...*indirect.h
  TplFnWithParameterPackUsingArgs1(ic, it);
  // IWYU: IndirectClass is...*indirect.h
  TplFnWithParameterPackUsingArgs2(ic);
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectTemplate is...*indirect.h
  TplFnWithParameterPackUsingArgs2(ic, it);
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: TplInI1 is...*-i1.h
  TplFnWithParameterPackUsingArgs2(ic, it, ti1);
  TplFnWithParameterPackNonUsingArgs(ic, it);
}

// Test handling class template parameter packs.

template <typename... Args>
struct TplStructWithParameterPack1 : Args... {};

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
// IWYU: TplInI1 needs a declaration
// IWYU: TplInI1 is...*-i1.h
TplStructWithParameterPack1<IndirectClass, TplInI1<int>> tswpp11;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
TplStructWithParameterPack1<IndirectClass> tswpp12;

// Test that IWYU doesn't crash.
TplStructWithParameterPack1<> tswpp13;

template <typename T, typename... Args>
struct TplStructWithParameterPack2 : T, Args... {};

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectTemplate is...*indirect.h
// IWYU: TplInI1 needs a declaration
// IWYU: TplInI1 is...*-i1.h
TplStructWithParameterPack2<IndirectClass, IndirectTemplate<int>, TplInI1<int>>
    // IWYU: IndirectClass is...*indirect.h
    // IWYU: IndirectTemplate is...*indirect.h
    // IWYU: TplInI1 is...*-i1.h
    tswpp21;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectTemplate is...*indirect.h
TplStructWithParameterPack2<IndirectClass, IndirectTemplate<int>> tswpp22;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
TplStructWithParameterPack2<IndirectClass> tswpp23;

// IWYU: IndirectClass is...*indirect.h
template <typename T = IndirectClass, typename... Args>
struct TplStructWithParameterPackAndDefArg : T, Args... {};

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectTemplate is...*indirect.h
TplStructWithParameterPackAndDefArg<IndirectClass, IndirectTemplate<int>>
    // IWYU: IndirectClass is...*indirect.h
    // IWYU: IndirectTemplate is...*indirect.h
    tswppda1;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
TplStructWithParameterPackAndDefArg<IndirectClass> tswppda2;

// Test that IWYU doesn't crash. There should be only one redecl of
// TplStructWithParameterPackAndDefArg to test it correctly.
// IWYU: IndirectClass needs a declaration
TplStructWithParameterPackAndDefArg<IndirectClass>* ptswppda2;

// TODO: probably, no need of IndirectClass here, because it is reported
// at template declaration.
// IWYU: IndirectClass is...*indirect.h
TplStructWithParameterPackAndDefArg<> tswppda3;

// IWYU: TplHost needs a declaration
// IWYU: TplHost is...*-i1.h
TplHost::InnerTpl<TplHost> inner_tpl;
// Test that IWYU doesn't scan the prefix of the template specialization type.
// No need of TplHost there because it was reported where explicitly written.
constexpr auto inner_tpl_size = sizeof(inner_tpl);

// ---------------------------------------------------------------

// Test provision of types by alias template declaration for template
// instantiation scan. The class template uses dereferenced parameter type so
// that there is no directly corresponding argument type for resugar_map, and
// the type provision info should be obtained by GetProvidedTypeComponents.

template <int>
// IWYU: IndirectClass is...*indirect.h
using ProvidingPtrAlias = IndirectClass*;

template <typename T>
struct UsingDereferenced {
  T ptr;
  static constexpr auto s = sizeof(*ptr);
};

UsingDereferenced<ProvidingPtrAlias<1>> udppa;
// IWYU: IndirectClass is...*indirect.h
UsingDereferenced<NonProvidingPtrAlias<1>> udnppa;

// ---------------------------------------------------------------

class Class;

template <typename T>
struct Host {
  template <typename>
  struct Nested1 {
    T t;
  };

  template <typename U>
  struct Nested2 {
    U u;
  };

  struct Intermediate1 {
    template <typename>
    struct Nested {
      T t;
    };
  };

  template <typename U>
  struct Level1 {
    template <typename>
    struct Level2 {
      T t;
      U u;
    };
  };

  typedef Level1<Class> Level1NonProviding;
  template <typename>
  using Level1NonProvidingAlTpl = Level1<Class>;

  template <typename>
  struct UsesInMethod {
    static void Fn() {
      T t;
    }
  };

  struct NestedNonTemplate {
    T t;
  };

  static void UseNestedNonTemplate() {
    (void)sizeof(NestedNonTemplate);
  }
};

// IWYU: IndirectClass needs a declaration
struct Derived : Host<IndirectClass> {
  // IWYU: IndirectClass needs a declaration
  using Host<IndirectClass>::NestedNonTemplate;
};

// IWYU: IndirectClass is...*indirect.h
using HostProvidingAlias = Host<IndirectClass>;

template <typename>
// IWYU: IndirectClass is...*indirect.h
using HostProvidingAliasTpl = Host<IndirectClass>;

using NestedWithClass = Host<Class>::Intermediate1;

template <typename T>
using AliasTplToTypedef = typename Host<T>::Level1NonProviding;

void TestMultiLevelArgs() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(Host<IndirectClass>::Nested1<int>);
  // IWYU: IndirectClass needs a declaration
  Host<IndirectClass>::Nested1<int>* nested11;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(*nested11);

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(Host<int>::Nested2<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  Host<int>::Nested2<IndirectClass>* nested2;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(*nested2);

  // IWYU: IndirectClass needs a declaration
  Host<IndirectClass>::Intermediate1::Nested<int>* inner_nested;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(*inner_nested);

  // IWYU: IndirectClass needs a declaration
  Host<int>::Level1<IndirectClass>::Level2<int>* level2;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(*level2);

  HostProvidingAlias::Nested1<int> hpan1;
  // IWYU: IndirectClass is...*indirect.h
  HostNonProvidingAlias::Nested1<int> hnpan1;

  HostProvidingAliasTpl<int>::Nested1<int> hpatn1;
  // IWYU: IndirectClass is...*indirect.h
  HostNonProvidingAliasTpl<int>::Nested1<int> hnpatn1;

  // IWYU: Class is...*-i1.h
  (void)sizeof(NestedWithClass::Nested<int>);
  NestedWithClass::Nested<int>* nested12;
  // IWYU: Class is...*-i1.h
  (void)sizeof(*nested12);

  // IWYU: IndirectClass needs a declaration
  // IWYU: Class is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  Host<IndirectClass>::Level1NonProviding::Level2<int> inner_nested2;
  // IWYU: IndirectClass needs a declaration
  Host<IndirectClass>::Level1NonProviding::Level2<int>* p_inner_nested2;
  // IWYU: Class is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(*p_inner_nested2);
  // IWYU: IndirectClass needs a declaration
  // IWYU: Class is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(Host<IndirectClass>::Level1NonProvidingAlTpl<int>::Level2<int>);

  // IWYU: IndirectClass needs a declaration
  // IWYU: Class is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(AliasTplToTypedef<IndirectClass>::Level2<int>);

  Host<Class>::Intermediate1 intermediate1;
  // IWYU: Class is...*-i1.h
  decltype(intermediate1)::Nested<int> inner_nested3;
  using Intermediate1AliasNonProviding = decltype(intermediate1);
  // IWYU: Class is...*-i1.h
  Intermediate1AliasNonProviding::Nested<int> inner_nested4;
  // IWYU: IndirectClass needs a declaration
  Host<IndirectClass>::UsesInMethod<int> uim;
  // IWYU: IndirectClass is...*indirect.h
  decltype(uim)::Fn();

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(Host<IndirectClass>::NestedNonTemplate);
  (void)sizeof(HostProvidingAlias::NestedNonTemplate);
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(HostNonProvidingAlias::NestedNonTemplate);
  // IWYU: IndirectClass needs a declaration
  Host<IndirectClass>::NestedNonTemplate* nnt;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(*nnt);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  Host<IndirectClass>::UseNestedNonTemplate();
  // Test using-type.
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(Derived::NestedNonTemplate);
}

// ---------------------------------------------------------------

struct ContainsIndirectClass {
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass ic;
};

template <typename>
class ContainsIndirectClassIndirectly {
  ContainsIndirectClass cic;
};

void TestNonTemplatesInsideTemplate() {
  // IndirectClass definition should not be reported here because
  // ContainsIndirectClass should already provide it.
  // IWYU: IndirectClass needs a declaration
  (void)sizeof(ContainsIndirectClassIndirectly<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  ContainsIndirectClassIndirectly<IndirectClass> ciciic;
}

// ---------------------------------------------------------------

/**** IWYU_SUMMARY

tests/cxx/template_args.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/template_args-i1.h"

tests/cxx/template_args.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- class Class;  // lines XX-XX

The full include-list for tests/cxx/template_args.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate
#include "tests/cxx/template_args-d1.h"  // for ProvidingAlias
#include "tests/cxx/template_args-d2.h"  // for HostNonProvidingAlias, HostNonProvidingAliasTpl, IndirectClassNonProviding, NonProvidingAlias, NonProvidingFunctionAlias1, NonProvidingFunctionAlias2, NonProvidingPtrAlias, TplWithMethodWithoutDefNonProviding
#include "tests/cxx/template_args-i1.h"  // for Class, TplHost, TplInI1
template <typename F> struct FunctionStruct;  // lines XX-XX

***** IWYU_SUMMARY */
