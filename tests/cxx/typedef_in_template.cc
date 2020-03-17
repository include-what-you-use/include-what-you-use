//===--- typedef_in_template.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/direct.h"
#include "tests/cxx/typedef_in_template-d1.h"

template<class T>
class Container {
 public:
  // Should not be an iwyu violation for T
  typedef T value_type;

  // C++11 alias declaration, should not be an iwyu violation for T
  using alias_type = T;

  // IWYU: Pair is...*typedef_in_template-i1.h
  typedef Pair<T,T> pair_type;
};


void Declarations() {
  // These do not need the full type for Class because they're template params.

  // TODO: This is almost certainly wrong, see bug #431
  // We should not require the full definition of Class for passing it as a
  // template argument, but we must require it when the typedef it's aliasing
  // is full-used.
  // The bug has instructions for how to provoke the error more obviously.

  // IWYU: Class needs a declaration
  Container<Class>::value_type vt;

  // IWYU: Class needs a declaration
  Container<Class>::pair_type pt;

  // IWYU: Class needs a declaration
  Container<Class>::alias_type at;
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

/**** IWYU_SUMMARY

tests/cxx/typedef_in_template.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/typedef_in_template-i1.h"

tests/cxx/typedef_in_template.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/typedef_in_template-d1.h"  // lines XX-XX

The full include-list for tests/cxx/typedef_in_template.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/typedef_in_template-i1.h"  // for Class (ptr only), Pair

***** IWYU_SUMMARY */
