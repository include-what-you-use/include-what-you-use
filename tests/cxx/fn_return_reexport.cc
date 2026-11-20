//===--- fn_return_reexport.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Wno-unused-value -Xiwyu --check_also=tests/cxx/*-direct.h

// Tests re-export policy for function return types.

#include "tests/cxx/fn_return_reexport-direct.h"

auto GetDeducedIndirectClass() {
  return GetProvidedIndirectClass();
}

auto get_indirect_class_lambda = [] { return GetProvidedIndirectClass(); };

// IWYU: IndirectClass needs a declaration
auto GetTrailingReturnType() -> IndirectClass;

template <typename T>
auto GetTrailingReturnTypeTpl() -> T;

// IWYU: Indirect needs a declaration
Indirect GetNonProvidedIndirect();
// IWYU: Indirect needs a declaration
const Indirect& GetNonProvidedIndirectConstRef();

// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectClass needs a declaration
const IndirectTemplate<IndirectClass>& GetNonProvidedTpl();

// IWYU: IndirectClass needs a declaration
using NonProvidingIndirectClassAlias = IndirectClass;

template <typename T>
struct Tpl {
  T GetT() const;
};

// IWYU: Indirect is...*-indirect.h
auto non_provided = GetNonProvidedIndirect();
auto provided = GetProvidedIndirect();

template <typename T>
void CallOverloadedMethod(T t) {
  // IWYU: Indirect is...*-indirect.h
  non_provided.OverloadedMethod(t);
  provided.OverloadedMethod(t);
}

void Test() {
  // There is no need to report IndirectClass here: it should be available
  // at the point of GetDeducedIndirectClass() and get_indirect_class_lambda
  // definitions, otherwise compilers could not deduce the return type.
  GetDeducedIndirectClass();
  get_indirect_class_lambda();
  // 'auto' with a trailing return type should not confuse IWYU: a user is
  // still responsible for the type info.
  // IWYU: IndirectClass is...*indirect.h
  GetTrailingReturnType();
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  GetTrailingReturnTypeTpl<IndirectClass>();

  // IWYU: Indirect is...*-indirect.h
  GetNonProvidedIndirect().Method();
  GetProvidedIndirect().Method();

  GetDeducedIndirectClass().Method();
  get_indirect_class_lambda().Method();

  // IWYU: IndirectClass is...*indirect.h
  GetTrailingReturnTypeTpl<NonProvidingIndirectClassAlias>().Method();
  GetTrailingReturnTypeTpl<ProvidingIndirectClassAlias>().Method();

  // IWYU: IndirectClass is...*indirect.h
  Tpl<NonProvidingIndirectClassAlias>{}.GetT().Method();
  Tpl<ProvidingIndirectClassAlias>{}.GetT().Method();

  // IWYU: Indirect is...*-indirect.h
  non_provided.Method();
  provided.Method();

  // IWYU: Indirect is...*-indirect.h
  (true ? non_provided : non_provided).Method();
  (true ? provided : provided).Method();
  (true ? provided : GetProvidedIndirect()).Method();
  // IWYU: Indirect is...*-indirect.h
  (non_provided ?: non_provided).Method();
  (provided ?: provided).Method();
  (provided ?: GetProvidedIndirect()).Method();

  // IWYU: Indirect is...*-indirect.h
  (provided, non_provided).Method();
  (non_provided, provided).Method();

  // IWYU: Indirect is...*-indirect.h
  for (auto i : non_provided)
    ;
  for (auto i : provided)
    ;

  // IWYU: Indirect is...*-indirect.h
  (&non_provided)[1];
  (&provided)[1];

  // IWYU: Indirect is...*-indirect.h
  &non_provided + 1;
  &provided + 1;

  // IWYU: Indirect is...*-indirect.h
  (&non_provided)[0].Method();
  (&provided)[0].Method();
  // IWYU: Indirect is...*-indirect.h
  0 [&non_provided].Method();
  0 [&provided].Method();

  // IWYU: Indirect is...*-indirect.h
  (*(&non_provided + 1 - 1)).Method();
  (*(&provided + 1 - 1)).Method();
  // IWYU: Indirect is...*-indirect.h
  (*(1 + &non_provided - 1)).Method();
  (*(1 + &provided - 1)).Method();

  const auto* non_provided_ptr = &non_provided;
  const auto* provided_ptr = &provided;
  // IWYU: Indirect is...*-indirect.h
  ((++non_provided_ptr)--)->Method();
  ((++provided_ptr)--)->Method();

  // IWYU: Indirect is...*-indirect.h
  sizeof(non_provided);
  sizeof(provided);

  // IWYU: Indirect is...*-indirect.h
  -non_provided;
  -provided;

  // IWYU: Indirect is...*-indirect.h
  auto non_provided_copy = non_provided;
  auto provided_copy = provided;
  // IWYU: Indirect is...*-indirect.h
  non_provided_copy.Method();
  provided_copy.Method();

  // IWYU: Indirect needs a declaration
  const Indirect& providing_init_expl_type_specified = GetProvidedIndirect();
  // Indirect should probably be reported here because it is explicitly written
  // in the variable type.
  // IWYU: Indirect is...*indirect.h
  providing_init_expl_type_specified.Method();

  const auto& provided_const_ref = GetProvidedIndirect();
  provided_const_ref.Method();

  const auto& non_provided_const_ref = GetNonProvidedIndirectConstRef();
  // Reporting Indirect at non_provided uses is moot because the type should be
  // already complete at non_provided variable declaration, but IWYU should
  // definitely report Indirect here:
  // IWYU: Indirect is...*indirect.h
  non_provided_const_ref.Method();

  // Require Indirect so as not to depend on header implementation details.
  // IWYU: Indirect is...*indirect.h
  hdr_provided.Method();

  // TODO: IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectTemplate is...*indirect.h
  IndirectTemplate non_provided_tpl = GetNonProvidedTpl();
  // IWYU: IndirectTemplate is...*indirect.h
  non_provided_tpl.Method();
  // IWYU: IndirectTemplate is...*indirect.h
  IndirectTemplate provided_tpl = GetProvidedTpl();
  provided_tpl.Method();

  IndirectClassIterator ici;
  // IWYU should not report IndirectClass because it is provided by the alias.
  ici->Method();
}

/**** IWYU_SUMMARY

tests/cxx/fn_return_reexport.cc should add these lines:
#include "tests/cxx/fn_return_reexport-indirect.h"
#include "tests/cxx/indirect.h"

tests/cxx/fn_return_reexport.cc should remove these lines:

The full include-list for tests/cxx/fn_return_reexport.cc:
#include "tests/cxx/fn_return_reexport-direct.h"  // for GetProvidedIndirect, GetProvidedIndirectClass, GetProvidedTpl, IndirectClassIterator, ProvidingIndirectClassAlias, hdr_provided
#include "tests/cxx/fn_return_reexport-indirect.h"  // for Indirect
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */
