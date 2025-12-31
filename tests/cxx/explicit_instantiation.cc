//===--- explicit_instantiation.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also=tests/cxx/explicit_instantiation-spec.h \
//            -Xiwyu --check_also=tests/cxx/explicit_instantiation-template.h \
//            -I .

#include "tests/cxx/explicit_instantiation-spec.h"
#include "tests/cxx/explicit_instantiation-template_direct.h"
#include "tests/cxx/explicit_instantiation-template2.h"

// The explicit instantiation of a specialization only needs a declaration
// of the base template.
template <>
// IWYU: Template needs a declaration
class Template<char*> {};
extern template class Template<char*>;

// Test that all explicit instantiations variants of the base template
// require the full type:

// - Declaration and definition
// IWYU: Template is...*explicit_instantiation-template.h
extern template class Template<int>;
// IWYU: Template is...*explicit_instantiation-template.h
template class Template<int>;

// - Only declaration
// IWYU: Template is...*explicit_instantiation-template.h
extern template class Template<short>;

// - Only definition
// IWYU: Template is...*explicit_instantiation-template.h
template class Template<double>;


// The explicit instantiation of a specialization only needs a declaration
// of the base template, but it should already be present because there are
// other Template specializations defined in this file above.
template<> class Template<char> {};
extern template class Template<char>;

// The partial specialization from 'explicit_instantiation-spec.h' is used here.
extern template class Template<int*>;
template class Template<int*>;

// IWYU: Template is...*explicit_instantiation-template.h
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
extern template class Template<IndirectClass>;
// IWYU: Template is...*explicit_instantiation-template.h
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
template class Template<IndirectClass>;

// Instantiation of class methods with explicit class instantiation definition
// (but not declaration) requires template argument type info for parameters
// used inside the methods.
// IWYU: ClassWithUsingMethod is...*explicit_instantiation-template.h
// IWYU: IndirectClass needs a declaration
extern template class ClassWithUsingMethod<IndirectClass>;
// One more instantiation declaration of the same specialization so as to test
// that method definitions to be scanned are taken from the correct declaration
// (clang places them in the first one).
// IWYU: ClassWithUsingMethod is...*explicit_instantiation-template.h
// IWYU: IndirectClass needs a declaration
extern template class ClassWithUsingMethod<IndirectClass>;
// IWYU: ClassWithUsingMethod is...*explicit_instantiation-template.h
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
template class ClassWithUsingMethod<IndirectClass>;

// Instantiation definition only.
// IWYU: ClassWithUsingMethod is...*explicit_instantiation-template.h
// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectTemplate is...*indirect.h
template class ClassWithUsingMethod<IndirectTemplate<int>>;

// The template argument is considered to be provided with type alias.
// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectTemplate is...*indirect.h
typedef IndirectTemplate<char> ProvidingTypedef;
// IWYU: ClassWithUsingMethod is...*explicit_instantiation-template.h
template class ClassWithUsingMethod<ProvidingTypedef>;

// The type template parameter is not used in a context requiring the full type
// info.
// IWYU: ClassWithMethodUsingPtr is...*explicit_instantiation-template.h
// IWYU: IndirectClass needs a declaration
template class ClassWithMethodUsingPtr<IndirectClass>;

// Test traversal of instantiated class methods when the instantiated template
// file is not analyzed (not included with 'check_also' command line option).
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
template class ClassWithUsingMethod2<IndirectClass>;

// IWYU should not forward-declare templates with default args. Test that
// an explicit instantiation after the template spec reference doesn't muss
// the tool.
// IWYU: TplWithDefArg is...*explicit_instantiation-template.h
TplWithDefArg<int>* p = nullptr;
// IWYU: TplWithDefArg<int, :0> is...*explicit_instantiation-template.h
extern template class TplWithDefArg<int>;

template <typename>
class TplUsingNondependentDecl {
  // IWYU: getInt() is...*explicit_instantiation-template.h
  static constexpr int i = getInt();
};

// No reporting getInt here.
template class TplUsingNondependentDecl<void>;

/**** IWYU_SUMMARY

tests/cxx/explicit_instantiation.cc should add these lines:
#include "tests/cxx/explicit_instantiation-template.h"
#include "tests/cxx/indirect.h"

tests/cxx/explicit_instantiation.cc should remove these lines:
- #include "tests/cxx/explicit_instantiation-template_direct.h"  // lines XX-XX

The full include-list for tests/cxx/explicit_instantiation.cc:
#include "tests/cxx/explicit_instantiation-spec.h"  // for Template
#include "tests/cxx/explicit_instantiation-template.h"  // for ClassWithMethodUsingPtr, ClassWithUsingMethod, Template, TplWithDefArg, getInt
#include "tests/cxx/explicit_instantiation-template2.h"  // for ClassWithUsingMethod2
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */
