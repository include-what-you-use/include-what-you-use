//===--- template_base.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests reporting types needed for class template base instantiation.

#include "tests/cxx/direct.h"

template <typename T>
struct DirectInheritance : T {
  struct Inner {};
};

template <typename T>
struct InheritanceFromTplInheritedFromT : DirectInheritance<T> {};

template <typename T>
struct TplInBaseNNS : InheritanceFromTplInheritedFromT<T>::Inner {};

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
constexpr auto s1 = sizeof(TplInBaseNNS<IndirectClass>);

template <typename T>
// TODO(bolshakov): forward-declaration is probably sufficient.
// IWYU: IndirectTemplate is...*indirect.h
struct InheritanceFromTplWithTMember : IndirectTemplate<T> {};

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
constexpr auto s2 = sizeof(InheritanceFromTplWithTMember<IndirectClass>);

template <typename T>
struct PtrOnly {
  T* t;
};

template <typename T>
struct InheritanceFromPtrOnly : PtrOnly<T> {};

// IWYU: IndirectClass needs a declaration
constexpr auto s3 = sizeof(InheritanceFromPtrOnly<IndirectClass>);

/**** IWYU_SUMMARY

tests/cxx/template_base.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/template_base.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/template_base.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */
