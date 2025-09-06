//===--- variable_template.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Wno-undefined-var-template \
//            -Xiwyu --check_also="tests/cxx/*-i1.h"

// Tests variable templates.

#include "tests/cxx/direct.h"
#include "tests/cxx/variable_template-d1.h"

class DeclaredInCC;
class IndirectClass;

using NonProvidingRefAlias = IndirectClass&;

template <typename>
int no_param_use = 0;

template <typename T>
T typed_as_param;

template <typename T>
T* typed_as_param_ptr;

template <typename T>
auto full_use_in_init = sizeof(T);

template <typename T>
auto ptr_use_in_init = sizeof(T*);

template <typename T>
int full_use_in_iife = [] {
  T t;
  return 1;
}();

template <typename T>
int ptr_use_in_iife = [] {
  T* p = nullptr;
  return 1;
}();

template <typename T = IndirectClass>
T full_use_def_arg_non_provided;

template <typename T1, typename T2 = IndirectClass>
int both_args_used_def_non_provided = [] {
  T1 t1;
  T2 t2;
  return 1;
}();

template <typename>
// IWYU: GetInt is...*-i1.h
int refers_to_fn_in_init = GetInt();

template <typename T>
T typed_as_param_expl_spec;

template <>
// IWYU: IndirectClass is...*indirect.h
IndirectClass typed_as_param_expl_spec<IndirectClass>;

template <typename T>
T typed_as_param_expl_inst_decl;

extern template IndirectClass typed_as_param_expl_inst_decl<IndirectClass>;

template <typename T>
T typed_as_param_expl_inst_def;

// TODO: IWYU: IndirectClass is...*indirect.h
template IndirectClass typed_as_param_expl_inst_def<IndirectClass>;

template <typename>
int full_use_in_partial_spec;

template <typename T>
int full_use_in_partial_spec<T*> = [] {
  T t;
  // IWYU: GetInt is...*-i1.h
  return GetInt();
}();

template <typename>
int ptr_use_in_partial_spec;

template <typename T>
int ptr_use_in_partial_spec<T*> = [] {
  T* t = nullptr;
  return 1;
}();

template <typename T>
extern T typed_as_param_extern;

template <typename T>
void TplFnUsingVarTpl() {
  (void)typed_as_param<T>;
}

template <typename T>
void TplFnUsingGetIntIndirectly() {
  (void)refers_to_fn_in_init<T>;
}

template <typename T>
void TplFnNoImplInstNeeded() {
  (void)typed_as_param_expl_spec<T>;
  (void)typed_as_param_expl_inst_decl<T>;
  (void)typed_as_param_expl_inst_def<T>;
  (void)typed_as_param_extern<T>;
}

void Fn() {
  (void)no_param_use<IndirectClass>;
  // IWYU: IndirectClass is...*indirect.h
  (void)typed_as_param<IndirectClass>;
  (void)typed_as_param_ptr<IndirectClass>;
  // IWYU: IndirectClass is...*indirect.h
  (void)full_use_in_init<IndirectClass>;
  // The complete referred-to type is needed due to sizeof.
  // IWYU: IndirectClass is...*indirect.h
  (void)full_use_in_init<IndirectClass&>;
  // IWYU: IndirectClass is...*indirect.h
  (void)full_use_in_init<NonProvidingRefAlias>;
  // IWYU: ProvidingRefAlias is...*-i1.h
  (void)full_use_in_init<ProvidingRefAlias>;
  (void)full_use_in_init<IndirectClass*>;
  (void)ptr_use_in_init<IndirectClass>;
  // IWYU: IndirectClass is...*indirect.h
  (void)full_use_in_iife<IndirectClass>;
  (void)ptr_use_in_iife<IndirectClass>;
  // IWYU: IndirectClass is...*indirect.h
  (void)full_use_def_arg_non_provided<>;
  (void)full_use_def_arg_non_provided<int>;
  // IWYU: full_use_def_arg_provided is...*-i1.h
  (void)full_use_def_arg_provided<>;
  // IWYU: full_use_def_arg_provided is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)full_use_def_arg_provided<IndirectClass>;
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  (void)both_args_used_def_non_provided<IndirectTemplate<int>>;
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectTemplate is...*indirect.h
  (void)both_args_used_def_non_provided<IndirectTemplate<int>, char>;
  // IWYU: both_args_used_def_provided is...*-i1.h
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectTemplate is...*indirect.h
  (void)both_args_used_def_provided<IndirectTemplate<int>>;
  // IWYU: both_args_used_def_provided is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectTemplate is...*indirect.h
  (void)both_args_used_def_provided<IndirectClass, IndirectTemplate<int>>;
  // Check that IWYU doesn't require GetInt here.
  (void)refers_to_fn_in_init<IndirectClass>;
  (void)typed_as_param_expl_spec<IndirectClass>;
  (void)typed_as_param_expl_inst_decl<IndirectClass>;
  (void)typed_as_param_expl_inst_def<IndirectClass>;
  // IWYU: IndirectClass is...*indirect.h
  (void)full_use_in_partial_spec<IndirectClass*>;
  (void)ptr_use_in_partial_spec<IndirectClass*>;
  (void)typed_as_param_extern<IndirectClass>;
  // IWYU: IndirectClass is...*indirect.h
  TplFnUsingVarTpl<IndirectClass>();
  TplFnUsingGetIntIndirectly<IndirectClass>();
  TplFnNoImplInstNeeded<IndirectClass>();
  // IWYU: UseVarTplInHeader is...*-i1.h
  UseVarTplInHeader<DeclaredInCC>();
}

/**** IWYU_SUMMARY

tests/cxx/variable_template.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/variable_template-i1.h"

tests/cxx/variable_template.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/variable_template-d1.h"  // lines XX-XX
- class IndirectClass;  // lines XX-XX

The full include-list for tests/cxx/variable_template.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate
#include "tests/cxx/variable_template-i1.h"  // for GetInt, ProvidingRefAlias, UseVarTplInHeader, both_args_used_def_provided, full_use_def_arg_provided
class DeclaredInCC;  // lines XX-XX

***** IWYU_SUMMARY */
