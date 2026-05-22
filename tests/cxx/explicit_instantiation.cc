//===--- explicit_instantiation.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also=tests/cxx/explicit_instantiation-spec-i1.h \
//            -Xiwyu --check_also=tests/cxx/explicit_instantiation-template.h \
//            -I . -Wno-instantiation-after-specialization

#include "tests/cxx/explicit_instantiation-spec-d1.h"
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
// IWYU: Template<:0 *> is...*explicit_instantiation-spec-i1.h
extern template class Template<int*>;
// IWYU: Template<:0 *> is...*explicit_instantiation-spec-i1.h
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

// IWYU: TplWithNotProvidedDefArg is...*explicit_instantiation-template.h
// IWYU: IndirectClass is...*indirect.h
extern template class TplWithNotProvidedDefArg<>;
extern template class TplWithProvidedDefArg<>;

// IWYU: IndirectTemplate needs a declaration
extern template class ClassWithUsingMethod2<IndirectTemplate<int>>;

// Test function template explicit instantiation.
// IWYU: TplFn() is...*explicit_instantiation-template.h
extern template void TplFn<int>();
// One more explicit instantiation declaration.
// IWYU: TplFn() is...*explicit_instantiation-template.h
extern template void TplFn<int>();
// IWYU: TplFn() is...*explicit_instantiation-template.h
template void TplFn<int>();

// IWYU: ClassWithUsingMethod is...*explicit_instantiation-template.h
// IWYU: IndirectTemplate needs a declaration
extern template void ClassWithUsingMethod<IndirectTemplate<float>>::Fn();
// IWYU: IndirectTemplate is...*indirect.h
// IWYU: IndirectTemplate needs a declaration
// IWYU: ClassWithUsingMethod is...*explicit_instantiation-template.h
template void ClassWithUsingMethod<IndirectTemplate<float>>::Fn();

// The presence of the explicit specialization should not affect explicit
// instantiation declarations: effect should be the same whether it is included
// or not, AFAIU.
// IWYU: TplFn() is...*explicit_instantiation-template.h
extern template void TplFn<char>();
// In the case of explicit instantiation definition, the explicit specialization
// should be available to suppress instantiation, so as to avoid ODR-violation.
// IWYU: TplFn() is...*explicit_instantiation-spec-i2.h
template void TplFn<char>();

template <typename>
void TplFnWithRedecl();

// An explicit instantiation declaration of a function template doesn't require
// the template definition. The redeclaration above is sufficient.
extern template void TplFnWithRedecl<int>();
// In the case of an explicit instantiation definition, the template definition
// is required.
// IWYU: TplFnWithRedecl() is...*explicit_instantiation-template.h
template void TplFnWithRedecl<int>();

// IWYU: TplFnWithRedecl2() is...*explicit_instantiation-template.h
extern template void TplFnWithRedecl2<int>();
// IWYU: TplFnWithRedecl2() is...*explicit_instantiation-template.h
template void TplFnWithRedecl2<int>();

template <typename>
void TplFnWithRedecl2();

// Test variable templates.
// IWYU: var_tpl is...*explicit_instantiation-template.h
extern template int var_tpl<int>;
// IWYU: var_tpl is...*explicit_instantiation-template.h
extern template int var_tpl<int>;
// IWYU: var_tpl is...*explicit_instantiation-template.h
template int var_tpl<int>;

// In the case of explicit specializations of variable templates, even explicit
// instantiation declaration depends on the specialization because the latter
// may change the variable type to some unrelated to the one of the primary
// template.
// IWYU: var_tpl<float> is...*explicit_instantiation-spec-i2.h
extern template char var_tpl<float>;
// IWYU: var_tpl<float> is...*explicit_instantiation-spec-i2.h
template char var_tpl<float>;
// IWYU: var_tpl<:0 *> is...*explicit_instantiation-spec-i2.h
extern template double var_tpl<int*>;
// IWYU: var_tpl<:0 *> is...*explicit_instantiation-spec-i2.h
template double var_tpl<int*>;

template <typename T>
extern int var_tpl_with_redecl;

extern template int var_tpl_with_redecl<int>;
// IWYU: var_tpl_with_redecl is...*explicit_instantiation-template.h
template int var_tpl_with_redecl<int>;

// IWYU: var_tpl_with_redecl2 is...*explicit_instantiation-template.h
extern template int var_tpl_with_redecl2<int>;
// IWYU: var_tpl_with_redecl2 is...*explicit_instantiation-template.h
template int var_tpl_with_redecl2<int>;

template <typename T>
extern int var_tpl_with_redecl2;

// IWYU: Host is...*explicit_instantiation-template.h
extern template void Host<int>::Fn();
// IWYU: Host is...*explicit_instantiation-template.h
// IWYU: Host::Fn() is...*explicit_instantiation-nested.h
template void Host<int>::Fn();

// IWYU: Host is...*explicit_instantiation-template.h
extern template void Host<int>::StaticFn();
// IWYU: Host is...*explicit_instantiation-template.h
// IWYU: Host::StaticFn() is...*explicit_instantiation-nested.h
template void Host<int>::StaticFn();

// IWYU: Host is...*explicit_instantiation-template.h
extern template int Host<int>::i;
// IWYU: Host is...*explicit_instantiation-template.h
// IWYU: Host::i is...*explicit_instantiation-nested.h
template int Host<int>::i;

// TODO: reporting '-nested.h' here is rather unwanted.
// IWYU: Host is...*explicit_instantiation-template.h
// IWYU: Host::var_tpl is...*explicit_instantiation-nested.h
extern template int Host<int>::var_tpl<int>;
// IWYU: Host is...*explicit_instantiation-template.h
// IWYU: Host::var_tpl is...*explicit_instantiation-nested.h
template int Host<int>::var_tpl<int>;
// '-nested.h' is required here because the explicit specialization changes
// the variable type.
// IWYU: Host is...*explicit_instantiation-template.h
// IWYU: Host::var_tpl<:0 *> is...*explicit_instantiation-nested.h
extern template char Host<int>::var_tpl<int*>;
// IWYU: Host is...*explicit_instantiation-template.h
// IWYU: Host::var_tpl<:0 *> is...*explicit_instantiation-nested.h
template char Host<int>::var_tpl<int*>;

void Fn() {
  // Test that reporting for the out-of-line method is blocked due to
  // the presence of ClassWithUsingMethod2 explicit instantiation declaration
  // above, but not for the inline method Fn.
  // IWYU: IndirectTemplate needs a declaration
  ClassWithUsingMethod2<IndirectTemplate<int>> cwum2iti;
  // IWYU: IndirectTemplate is...*indirect.h
  cwum2iti.Fn();
  cwum2iti.OutOfLineMethod();
}

/**** IWYU_SUMMARY

tests/cxx/explicit_instantiation.cc should add these lines:
#include "tests/cxx/explicit_instantiation-nested.h"
#include "tests/cxx/explicit_instantiation-spec-i1.h"
#include "tests/cxx/explicit_instantiation-spec-i2.h"
#include "tests/cxx/explicit_instantiation-template.h"
#include "tests/cxx/indirect.h"

tests/cxx/explicit_instantiation.cc should remove these lines:
- #include "tests/cxx/explicit_instantiation-spec-d1.h"  // lines XX-XX
- #include "tests/cxx/explicit_instantiation-template_direct.h"  // lines XX-XX

The full include-list for tests/cxx/explicit_instantiation.cc:
#include "tests/cxx/explicit_instantiation-nested.h"  // for Host::Fn, Host::StaticFn, Host::i, Host::var_tpl
#include "tests/cxx/explicit_instantiation-spec-i1.h"  // for Template
#include "tests/cxx/explicit_instantiation-spec-i2.h"  // for TplFn, var_tpl
#include "tests/cxx/explicit_instantiation-template.h"  // for ClassWithMethodUsingPtr, ClassWithUsingMethod, Host, Template, TplFn, TplFnWithRedecl, TplFnWithRedecl2, TplWithDefArg, TplWithNotProvidedDefArg, getInt, var_tpl, var_tpl_with_redecl, var_tpl_with_redecl2
#include "tests/cxx/explicit_instantiation-template2.h"  // for ClassWithUsingMethod2, TplWithProvidedDefArg
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */
