//===--- specialization_needs_decl.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that when we specialize a template, we require a primary template
// declaration to be available. However, forward-declaring a specialization
// doesn't require a definition.

#include "tests/cxx/specialization_needs_decl-d1.h"

template<> struct TplStruct<int> { };

// Forward-declaring a specialization is the same: we don't require
// the definition.
template<> struct TplStruct<float>;

// Full-using a specialization requires definition of the specialization to be
// included. Not the base template.

// IWYU: Template needs a declaration
int f(Template<int>& t) {
  // IWYU: Template<int> is...*specialization_needs_decl-i1.h
  return t.x;
}

// Function template specialization requires a primary template declaration.
template <>
// IWYU: TplFn1 is...*specialization_needs_decl-i1.h
void TplFn1<int>();

template <typename>
void TplFn2();
// A primary template redeclaration is already present in this file.
template <>
void TplFn2<int>();

// The primary template redeclaration from this file is irrelevant because it is
// after the specialization.
template <>
// IWYU: TplFn3 is...*specialization_needs_decl-i1.h
void TplFn3<int>();
template <typename>
void TplFn3();

// Variable template specializations require a primary template declaration.
template <>
// IWYU: var_tpl1 is...*specialization_needs_decl-i1.h
int var_tpl1<char>;

template <typename T>
// IWYU: var_tpl1 is...*specialization_needs_decl-i1.h
int var_tpl1<T*>;

template <typename>
extern int var_tpl2;
// A primary template redeclaration is already present in this file.
template <>
int var_tpl2<char>;

// The primary template redeclaration from this file is irrelevant because it is
// after the specialization.
template <>
// IWYU: var_tpl3 is...*specialization_needs_decl-i1.h
int var_tpl3<char>;
template <typename>
extern int var_tpl3;

/**** IWYU_SUMMARY

tests/cxx/specialization_needs_decl.cc should add these lines:
#include "tests/cxx/specialization_needs_decl-i1.h"
template <typename T> struct TplStruct;

tests/cxx/specialization_needs_decl.cc should remove these lines:
- #include "tests/cxx/specialization_needs_decl-d1.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_needs_decl.cc:
#include "tests/cxx/specialization_needs_decl-i1.h"  // for Template, TplFn1, TplFn3, var_tpl1, var_tpl3
template <typename T> struct TplStruct;

***** IWYU_SUMMARY */
