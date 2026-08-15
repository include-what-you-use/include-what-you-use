//===--- typedef_in_template.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++11 -I . \
//            -Xiwyu --check_also=tests/cxx/typedef_in_template-d3.h

#include "tests/cxx/direct.h"
#include "tests/cxx/typedef_in_template-d1.h"
#include "tests/cxx/typedef_in_template-d2.h"
#include "tests/cxx/typedef_in_template-d3.h"

template<class T1, class T2>
class Container {
 public:
  // Should not be an iwyu violation for T1
  typedef T1 value_type;

  // C++11 alias declaration, should not be an iwyu violation for T1
  using alias_type = T1;

  // IWYU: Pair needs a declaration
  typedef Pair<T2,T2> pair_type;
};


void Declarations() {
  // Just using Container does not need the full types because there are only
  // aliases made, which do not require full-uses.

  // IWYU: Class1 needs a declaration
  // IWYU: Class2 needs a declaration
  Container<Class1, Class2> c;

  // Full-using any of those aliases *should* require a full use
  // of corresponding template argument type.

  // IWYU: Class1 is...*typedef_in_template-i1.h
  // IWYU: Class1 needs a declaration
  // IWYU: Class2 needs a declaration
  Container<Class1, Class2>::value_type vt;

  // IWYU: Class1 needs a declaration
  // IWYU: Class2 is...*typedef_in_template-i2.h
  // IWYU: Class2 needs a declaration
  // IWYU: Pair is...*typedef_in_template-i2.h
  Container<Class1, Class2>::pair_type pt;

  // IWYU: Class1 is...*typedef_in_template-i1.h
  // IWYU: Class1 needs a declaration
  // IWYU: Class2 needs a declaration
  Container<Class1, Class2>::alias_type at;

  // IWYU: Class1 needs a declaration
  // IWYU: Class2 needs a declaration
  HeaderContainer<Class1, Class2> hc;

  // IWYU: Class1 is...*typedef_in_template-i1.h
  // IWYU: Class1 needs a declaration
  // IWYU: Class2 needs a declaration
  HeaderContainer<Class1, Class2>::value_type hvt;

  // IWYU: Class1 needs a declaration
  // IWYU: Class2 is...*typedef_in_template-i2.h
  // IWYU: Class2 needs a declaration
  HeaderContainer<Class1, Class2>::pair_type hpt;

  // IWYU: Class1 is...*typedef_in_template-i1.h
  // IWYU: Class1 needs a declaration
  // IWYU: Class2 needs a declaration
  HeaderContainer<Class1, Class2>::alias_type hat;
}

// STL containers are often implemented via a complex web of type aliases and
// helper classes.  Tracking uses through all these layers can be non-trivial.
// The following are some reduced examples in roughly increasing order of
// complexity which can serve as helpful test cases while debugging such
// issues.  They were inspired by libstdc++'s implementation of
// std::unordered_map, but don't directly correspond to it.

// Verify that a full-use of an alias of a template parameter is treated as a
// full-use of that parameter.
template <typename T>
struct UsesAliasedParameter {
  using TAlias = T;
  TAlias t;
};

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
UsesAliasedParameter<IndirectClass> a;
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
constexpr auto s1 = sizeof(UsesAliasedParameter<IndirectClass>);

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
UsesAliasedParameter<IndirectClass>::TAlias a2;

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Header_UsesAliasedParameter<IndirectClass> ha;
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
constexpr auto hs1 = sizeof(Header_UsesAliasedParameter<IndirectClass>);

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Header_UsesAliasedParameter<IndirectClass>::TAlias ha2;

// Try a more complex example, through an additional layer of indirection.
template <typename T>
struct IndirectlyUsesAliasedParameter {
  using TAlias = typename UsesAliasedParameter<T>::TAlias;
  TAlias t;
};

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
IndirectlyUsesAliasedParameter<IndirectClass> b;

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Header_IndirectlyUsesAliasedParameter<IndirectClass> hb;

template <typename T>
struct NestedUseOfAliasedParameter {
  using UserAlias = UsesAliasedParameter<T>;
  UserAlias a;
};

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
NestedUseOfAliasedParameter<IndirectClass> c;

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Header_NestedUseOfAliasedParameter<IndirectClass> hc;

template <typename T>
struct UsesAliasedSugaredParameter {
  static T t1;
  using TAlias = decltype(t1);
  TAlias t2;
};

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
constexpr auto s2 = sizeof(UsesAliasedSugaredParameter<IndirectClass>);

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
constexpr auto hs2 = sizeof(Header_UsesAliasedSugaredParameter<IndirectClass>);

// Passing aliased type as a template argument.

template <typename T>
struct Identity {
  using Type = T;

  struct Inner {
    using Type = T;
  };

  static T t;
  using SugaredType = decltype(t);

  template <int>
  using AliasTemplate = T;
};

template <typename T>
struct Outer {
  template <typename U>
  struct Inner {
    // IWYU: Pair needs a declaration
    using AliasedTpl = Pair<T, U>;
  };
};

struct ConstructionFromIndirectClass {
  // IWYU: IndirectClass needs a declaration
  ConstructionFromIndirectClass(IndirectClass);
};

struct AggregateContainingIndirectClass {
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass ic;
};

template <int>
using ProvidingLocalAliasTpl = ProvidingNested;
using ProvidingLocal = ProvidingLocalAliasTpl<1>;

void ArgumentTypeProvision() {
  Identity<Providing>::Type p1;
  (void)sizeof(p1);
  p1.Method();
  Identity<Providing>::Type* p2 = nullptr;
  (void)sizeof(*p2);
  p2->Method();
  decltype(p1) decltype_provided;
  (void)sizeof(decltype_provided);
  ConstructionFromIndirectClass cficp{decltype_provided};
  AggregateContainingIndirectClass acicp{decltype_provided};

  // IWYU: NonProviding is...*typedef_in_template-i1.h
  // IWYU: IndirectClass is...*indirect.h
  Identity<NonProviding>::Type n1;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(n1);
  // IWYU: IndirectClass is...*indirect.h
  n1.Method();
  // IWYU: NonProviding is...*typedef_in_template-i1.h
  Identity<NonProviding>::Type* n2 = nullptr;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(*n2);
  // IWYU: IndirectClass is...*indirect.h
  n2->Method();
  // IWYU: IndirectClass is...*indirect.h
  decltype(n1) decltype_not_provided;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(decltype_not_provided);
  // IWYU: IndirectClass is...*indirect.h
  ConstructionFromIndirectClass cficnp{decltype_not_provided};
  AggregateContainingIndirectClass acicnp{decltype_not_provided};

  Identity<Providing>::Inner::Type p3;

  // IWYU: Pair is...*typedef_in_template-i2.h
  Outer<Providing>::Inner<Providing>::AliasedTpl pp;

  // IWYU: IndirectClass needs a declaration
  // IWYU: Pair is...*typedef_in_template-i2.h
  Outer<Providing>::Inner<IndirectClass*>::AliasedTpl p_ptr;
  // IWYU: IndirectClass needs a declaration
  // IWYU: Pair is...*typedef_in_template-i2.h
  Outer<IndirectClass*>::Inner<Providing>::AliasedTpl ptr_p;

  HeaderOuter<Providing>::Inner<Providing>::AliasedTpl hpp;

  // IWYU: IndirectClass needs a declaration
  HeaderOuter<Providing>::Inner<IndirectClass*>::AliasedTpl hp_ptr;
  // IWYU: IndirectClass needs a declaration
  HeaderOuter<IndirectClass*>::Inner<Providing>::AliasedTpl hptr_p;

  Identity<Providing>::AliasTemplate<1> atp;
  (void)sizeof(Identity<Providing>::AliasTemplate<1>);
  // IWYU: NonProviding is...*typedef_in_template-i1.h
  // IWYU: IndirectClass is...*indirect.h
  Identity<NonProviding>::AliasTemplate<1> atn;
  // IWYU: NonProviding is...*typedef_in_template-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(Identity<NonProviding>::AliasTemplate<1>);

  HeaderIdentity<Providing>::AliasTemplate<1> hatp;
  (void)sizeof(HeaderIdentity<Providing>::AliasTemplate<1>);
  // IWYU: NonProviding is...*typedef_in_template-i1.h
  // IWYU: IndirectClass is...*indirect.h
  HeaderIdentity<NonProviding>::AliasTemplate<1> hatn;
  // IWYU: NonProviding is...*typedef_in_template-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(HeaderIdentity<NonProviding>::AliasTemplate<1>);

  ProvidingNested::Type pnt;
  // IWYU: NonProvidingNested is...*typedef_in_template-i1.h
  // IWYU: IndirectClass is...*indirect.h
  NonProvidingNested::Type nnt;
  ProvidingLocal::Type plt;

  TplProvidingDefArg<>::ArgType tpdaat;
}

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
constexpr auto s3 = sizeof(Identity<IndirectClass>::SugaredType);

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
constexpr auto hs3 = sizeof(HeaderIdentity<IndirectClass>::SugaredType);

// Test handling some of builtin unary transforms in dependent typedefs.

void TestUnaryTransformTypes() {
  // IWYU: IndirectClass needs a declaration
  UnaryTransformTypes<IndirectClass>::AddPointer p = nullptr;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(*p);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(UnaryTransformTypes<IndirectClass[2][3]>::RemoveAllExtents);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(UnaryTransformTypes<IndirectClass*>::RemovePointer);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(UnaryTransformTypes<IndirectClass&>::RemoveReference);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(UnaryTransformTypes<IndirectClass>::Identity);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(UnaryTransformTypes<IndirectClass>::PairAlias1);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(UnaryTransformTypes<IndirectClass>::PairAlias2);

  (void)sizeof(NonDependentUnaryTransformTypes::RemoveAllExtents);
  (void)sizeof(NonDependentUnaryTransformTypes::RemovePointer);
  (void)sizeof(NonDependentUnaryTransformTypes::RemoveReference);
  (void)sizeof(NonDependentUnaryTransformTypes::DummyRemoveReference);
}

/**** IWYU_SUMMARY

tests/cxx/typedef_in_template.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/typedef_in_template-i1.h"
#include "tests/cxx/typedef_in_template-i2.h"

tests/cxx/typedef_in_template.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/typedef_in_template-d2.h"  // lines XX-XX

The full include-list for tests/cxx/typedef_in_template.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/typedef_in_template-d1.h"  // for TplProvidingDefArg
#include "tests/cxx/typedef_in_template-d3.h"  // for HeaderContainer, HeaderIdentity, HeaderOuter, Header_IndirectlyUsesAliasedParameter, Header_NestedUseOfAliasedParameter, Header_UsesAliasedParameter, Header_UsesAliasedSugaredParameter, NonDependentUnaryTransformTypes, Providing, ProvidingNested, UnaryTransformTypes
#include "tests/cxx/typedef_in_template-i1.h"  // for Class1, NonProviding, NonProvidingNested
#include "tests/cxx/typedef_in_template-i2.h"  // for Class2, Pair

***** IWYU_SUMMARY */
