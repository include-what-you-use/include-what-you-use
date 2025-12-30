//===--- fwd_decl_print-d2.h - test input file for iwyu -- C++ ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/fwd_decl_print-i1.h"

// Simple templates in global namespace.
template <class T>
class Template {};

// Simple templates types in user-defined namespace.
namespace ns1 {
template <class T>
class TemplateInNamespace {};
}

// Derived template types.
template<class T>
class DerivedTemplate : public Template<T> {};

// Templates with attributes.
template <class T>
class [[nodiscard]] TemplateWithStdAttr {};

template <class T>
class __attribute__((warn_unused_result)) TemplateWithGNUAttr {};

// Templates with implicit attributes.
template <class T>
class alignas(8) TemplateWithAlignAs {};

template <class T>
class TemplateWithFinal final {};

// Templates with requires constraints
template <typename T>
requires requires(T a) { a; }
class TemplateWithSimpleAdHocRequires {};

template <typename T>
requires requires(T a) {
  // Use *, >, ? and : to set traps for any attempts at parsing.
  sizeof(decltype(*a)) > 6 ? true : false;
  a - a;
  &a;
}
class TemplateWithComplexAdHocRequires {};

// Use Addable from fwd_decl_print-d3.h
template <typename T>
requires Addable<T>
class TemplateWithRequiresClause {};
