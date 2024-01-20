//===--- typedef_in_template.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++11 -I .

#include "tests/cxx/direct.h"
#include "tests/cxx/typedef_in_template-d1.h"
#include "tests/cxx/typedef_in_template-d2.h"

template<class T1, class T2>
class Container {
 public:
  // Should not be an iwyu violation for T1
  typedef T1 value_type;

  // C++11 alias declaration, should not be an iwyu violation for T1
  using alias_type = T1;

  // IWYU: Pair needs a declaration
  // IWYU: Pair is...*typedef_in_template-i2.h
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
  Container<Class1, Class2>::pair_type pt;

  // IWYU: Class1 is...*typedef_in_template-i1.h
  // IWYU: Class1 needs a declaration
  // IWYU: Class2 needs a declaration
  Container<Class1, Class2>::alias_type at;
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

// Try a more complex example, through an additional layer of indirection.
template <typename T>
struct IndirectlyUsesAliasedParameter {
  using TAlias = typename UsesAliasedParameter<T>::TAlias;
  TAlias t;
};

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
IndirectlyUsesAliasedParameter<IndirectClass> b;

template <typename T>
struct NestedUseOfAliasedParameter {
  using UserAlias = UsesAliasedParameter<T>;
  UserAlias a;
};

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
NestedUseOfAliasedParameter<IndirectClass> c;

template <typename T>
struct UsesAliasedSugaredParameter {
  static T t1;
  using TAlias = decltype(t1);
  TAlias t2;
};

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
constexpr auto s2 = sizeof(UsesAliasedSugaredParameter<IndirectClass>);

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
    // IWYU: Pair is...*typedef_in_template-i2.h
    using AliasedTpl = Pair<T, U>;
  };
};

// IWYU: IndirectClass is...*indirect.h
using Providing = IndirectClass;

void ArgumentTypeProvision() {
  Identity<Providing>::Type p1;
  (void)sizeof(p1);
  p1.Method();
  Identity<Providing>::Type* p2 = nullptr;
  (void)sizeof(*p2);
  p2->Method();

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

  Identity<Providing>::Inner::Type p3;

  Outer<Providing>::Inner<Providing>::AliasedTpl pp;

  // IWYU: IndirectClass needs a declaration
  Outer<Providing>::Inner<IndirectClass*>::AliasedTpl p_ptr;
  // IWYU: IndirectClass needs a declaration
  Outer<IndirectClass*>::Inner<Providing>::AliasedTpl ptr_p;

  Identity<Providing>::AliasTemplate<1> atp;
  (void)sizeof(Identity<Providing>::AliasTemplate<1>);
  // IWYU: NonProviding is...*typedef_in_template-i1.h
  // IWYU: IndirectClass is...*indirect.h
  Identity<NonProviding>::AliasTemplate<1> atn;
  // IWYU: NonProviding is...*typedef_in_template-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(Identity<NonProviding>::AliasTemplate<1>);
}

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
constexpr auto s3 = sizeof(Identity<IndirectClass>::SugaredType);

/**** IWYU_SUMMARY

tests/cxx/typedef_in_template.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/typedef_in_template-i1.h"
#include "tests/cxx/typedef_in_template-i2.h"

tests/cxx/typedef_in_template.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/typedef_in_template-d1.h"  // lines XX-XX
- #include "tests/cxx/typedef_in_template-d2.h"  // lines XX-XX

The full include-list for tests/cxx/typedef_in_template.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/typedef_in_template-i1.h"  // for Class1, NonProviding
#include "tests/cxx/typedef_in_template-i2.h"  // for Class2, Pair

***** IWYU_SUMMARY */
