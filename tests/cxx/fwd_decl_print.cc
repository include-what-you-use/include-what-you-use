//===--- fwd_decl_print.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++20

// Test the printing of forward-declare lines for forward-declarable types. Does
// not attempt to test the rules or semantics of when uses are
// forward-declarable, only how they render when they are.

#include "tests/cxx/fwd_decl_print-d1.h"
#include "tests/cxx/fwd_decl_print-d2.h"

// Simple tag types.
Enum* e1;
Class* c1;
Struct* s1;

ns1::EnumInNamespace* e2;
ns1::ClassInNamespace* c2;
ns1::StructInNamespace* s2;

// Derived class types.
DerivedClass* dc1;

// Class types with explicit and implicit attributes (attrs should not be
// included in fwd decl).
ClassWithStdAttr* ac1;
ClassWithGNUAttr* ac2;
// alignas can be included, but doesn't have to.
ClassWithAlignAs* ac3;
ClassWithFinal* ac4;

// Simple templates.
Template<int>* t1;
ns1::TemplateInNamespace<char>* t2;

// Derived templates
DerivedTemplate<int>* dt1;

// Templates with explicit and implicit attributes (attrs should not be included
// in fwd decl).
TemplateWithStdAttr<int>* at1;
TemplateWithGNUAttr<int>* at2;
// alignas can be included, but doesn't have to.
// TODO: make this consistent between tag types and templates; we currently
// print alignas for templates. Should probably not.
TemplateWithAlignAs<int>* at3;
TemplateWithFinal<int>* at4;

// Templates with requires constraints.
TemplateWithSimpleAdHocRequires<void**>* rt1;
TemplateWithComplexAdHocRequires<void**>* rt2;

// TODO: the forward-declare for this requires Addable. We don't understand
// enough of concepts yet, but maybe a template with a requires clause with
// additional dependencies should simply not be forward-declarable.
TemplateWithRequiresClause<int>* rt3;

/**** IWYU_SUMMARY

tests/cxx/fwd_decl_print.cc should add these lines:
class Class;
class ClassWithAlignAs;
class ClassWithFinal;
class ClassWithGNUAttr;
class ClassWithStdAttr;
class DerivedClass;
enum Enum : int;
namespace ns1 { class ClassInNamespace; }
namespace ns1 { enum EnumInNamespace : int; }
namespace ns1 { struct StructInNamespace; }
namespace ns1 { template <class T> class TemplateInNamespace; }
struct Struct;
template <class T> class DerivedTemplate;
template <class T> class Template;
template <class T> class TemplateWithFinal;
template <class T> class TemplateWithGNUAttr;
template <class T> class TemplateWithStdAttr;
template <class T> class alignas(8)  TemplateWithAlignAs;
template <typename T> requires Addable<T> class TemplateWithRequiresClause;
template <typename T> requires requires (T a) { a; } class TemplateWithSimpleAdHocRequires;
template <typename T> requires requires (T a) { sizeof(decltype(*a)) > 6 ? true : false; a - a; &a; } class TemplateWithComplexAdHocRequires;

tests/cxx/fwd_decl_print.cc should remove these lines:
- #include "tests/cxx/fwd_decl_print-d1.h"  // lines XX-XX
- #include "tests/cxx/fwd_decl_print-d2.h"  // lines XX-XX

The full include-list for tests/cxx/fwd_decl_print.cc:
class Class;
class ClassWithAlignAs;
class ClassWithFinal;
class ClassWithGNUAttr;
class ClassWithStdAttr;
class DerivedClass;
enum Enum : int;
namespace ns1 { class ClassInNamespace; }
namespace ns1 { enum EnumInNamespace : int; }
namespace ns1 { struct StructInNamespace; }
namespace ns1 { template <class T> class TemplateInNamespace; }
struct Struct;
template <class T> class DerivedTemplate;
template <class T> class Template;
template <class T> class TemplateWithFinal;
template <class T> class TemplateWithGNUAttr;
template <class T> class TemplateWithStdAttr;
template <class T> class alignas(8)  TemplateWithAlignAs;
template <typename T> requires Addable<T> class TemplateWithRequiresClause;
template <typename T> requires requires (T a) { a; } class TemplateWithSimpleAdHocRequires;
template <typename T> requires requires (T a) { sizeof(decltype(*a)) > 6 ? true : false; a - a; &a; } class TemplateWithComplexAdHocRequires;

***** IWYU_SUMMARY */
