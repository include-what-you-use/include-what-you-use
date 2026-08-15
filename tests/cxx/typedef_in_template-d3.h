//===--- typedef_in_template-d3.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/direct.h"
#include "tests/cxx/typedef_in_template-d1.h"
#include "tests/cxx/typedef_in_template-d2.h"

template <typename T>
struct Identity;

// Some cases from typedef_in_template.cc have been duplicated here to test them
// with a different provision policy (only typedefs from headers can provide
// their underlying types).

template <class T1, class T2>
class HeaderContainer {
 public:
  // Should not be an iwyu violation for T1
  typedef T1 value_type;

  // C++11 alias declaration, should not be an iwyu violation for T1
  using alias_type = T1;

  // IWYU: Pair needs a declaration
  // IWYU: Pair is...*typedef_in_template-i2.h
  typedef Pair<T2, T2> pair_type;
};

template <typename T>
struct Header_UsesAliasedParameter {
  using TAlias = T;
  TAlias t;
};

template <typename T>
struct Header_IndirectlyUsesAliasedParameter {
  using TAlias = typename Header_UsesAliasedParameter<T>::TAlias;
  TAlias t;
};

template <typename T>
struct Header_NestedUseOfAliasedParameter {
  using UserAlias = Header_UsesAliasedParameter<T>;
  UserAlias a;
};

template <typename T>
struct Header_UsesAliasedSugaredParameter {
  static T t1;
  using TAlias = decltype(t1);
  TAlias t2;
};

// IWYU: IndirectClass is...*indirect.h
using Providing = IndirectClass;

// This alias provides IndirectClass.
// IWYU: IndirectClass is...*indirect.h
using ProvidingNested = Identity<IndirectClass>;

template <typename T>
struct HeaderIdentity {
  static T t;
  using SugaredType = decltype(t);

  template <int>
  using AliasTemplate = T;
};

template <typename T>
struct HeaderOuter {
  template <typename U>
  struct Inner {
    // IWYU: Pair needs a declaration
    // IWYU: Pair is...*typedef_in_template-i2.h
    using AliasedTpl = Pair<T, U>;
  };
};

template <typename T>
struct UnaryTransformTypes {
  using AddPointer = __add_pointer(T);
  using RemoveAllExtents = __remove_all_extents(T);
  using RemovePointer = __remove_pointer(T);
  using RemoveReference = __remove_reference_t(T);
  using Identity = __remove_reference_t(T&);
  // IWYU: Pair needs a declaration
  // IWYU: Pair is...*typedef_in_template-i2.h
  // IWYU: Class1 is...*typedef_in_template-i1.h
  using PairAlias1 = __remove_pointer(Pair<Class1, T>*);
  // IWYU: Pair needs a declaration
  // IWYU: Pair is...*typedef_in_template-i2.h
  // IWYU: Class1 is...*typedef_in_template-i1.h
  using PairAlias2 = __remove_pointer(Pair<Class1, T>);
};

struct NonDependentUnaryTransformTypes {
  // IWYU: IndirectClass is...*indirect.h
  using RemoveAllExtents = __remove_all_extents(IndirectClass[2][3]);
  // IWYU: IndirectClass is...*indirect.h
  using RemovePointer = __remove_pointer(IndirectClass*);
  // IWYU: IndirectClass is...*indirect.h
  using RemoveReference = __remove_reference_t(IndirectClass&);
  // IWYU: IndirectClass is...*indirect.h
  using DummyRemoveReference = __remove_reference_t(IndirectClass);
};

/**** IWYU_SUMMARY

tests/cxx/typedef_in_template-d3.h should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/typedef_in_template-i1.h"
#include "tests/cxx/typedef_in_template-i2.h"

tests/cxx/typedef_in_template-d3.h should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/typedef_in_template-d1.h"  // lines XX-XX
- #include "tests/cxx/typedef_in_template-d2.h"  // lines XX-XX

The full include-list for tests/cxx/typedef_in_template-d3.h:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/typedef_in_template-i1.h"  // for Class1
#include "tests/cxx/typedef_in_template-i2.h"  // for Pair
template <typename T> struct Identity;  // lines XX-XX+1

***** IWYU_SUMMARY */
