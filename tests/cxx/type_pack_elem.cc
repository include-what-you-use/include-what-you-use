//===--- type_pack_elem.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that __type_pack_element (used in standard library implementations of
// std::tuple) types are recognized and reported.

#include "tests/cxx/direct.h"

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
__type_pack_element<0, IndirectClass>
    // IWYU: IndirectClass is...*indirect.h
    val;
// IWYU: IndirectClass is...*indirect.h
constexpr auto s = sizeof(val);

// IWYU: IndirectClass needs a declaration
__type_pack_element<0, IndirectClass>* ptr = nullptr;

// Member-exprs
// IWYU: IndirectClass is...*indirect.h
__type_pack_element<0, decltype(IndirectClass::a)>
    member_expr_val;

// Full type is needed even in fwd-decl context.
// IWYU: IndirectClass is...*indirect.h
__type_pack_element<0, decltype(IndirectClass::a)>*
    member_expr_ptr;

// Provided types
// IWYU: IndirectClass is...*indirect.h
using Providing = IndirectClass;

__type_pack_element<0, Providing>
    type_is_provided_by_arg;

// Test that we see through __type_pack_element alias templates and handle
// aliased template specialization.
template<class T> struct FullUseTemplateArgInSizeof {
  char argument[sizeof(T)];
};

template <class T> using Alias = FullUseTemplateArgInSizeof<T>;
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
__type_pack_element<0, Alias<IndirectClass>>
    // IWYU: IndirectClass is...*indirect.h
    alias;

// Template nesting
template <class T>
struct Tpl {
  T t;
};

// Here is how Tpl itself behaves when nested...
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Tpl<Tpl<IndirectClass>>
    // IWYU: IndirectClass is...*indirect.h
    t0;

// ... and it behaves the same when nested inside __type_pack_element.
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
__type_pack_element<0, Tpl<Tpl<IndirectClass>>>
    // IWYU: IndirectClass is...*indirect.h
    t1;

void Tuple1617() {
  // This is a reduction of the std::tuple, type deduction and structured
  // bindings example in issue 1617.

  // IWYU: IndirectClass needs a declaration
  auto x = __type_pack_element<0, IndirectClass*>{};
  // IWYU: IndirectClass is...*indirect.h
  x->Method();
}

/**** IWYU_SUMMARY

tests/cxx/type_pack_elem.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/type_pack_elem.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/type_pack_elem.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
