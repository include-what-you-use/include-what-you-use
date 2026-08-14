//===--- static_data_member.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --mapping_file=tests/cxx/static_data_member.imp \
//            -Xiwyu --check_also=tests/cxx/*-i1.h

// Tests handling static data members, especially in templates.

#include "tests/cxx/static_data_member-d1.h"

void SetI() {
  // IWYU: Tpl is...*-i2.h
  // IWYU: Tpl::i is...*-i1.h
  Tpl<int>::i = 1;
  // No need to report IndirectClass here.
  // IWYU: Tpl is...*-i2.h
  // IWYU: Tpl<char>::i is...*-i1.h
  Tpl<char>::i = 1;
  // IWYU: PartiallySpecializedTpl<:0 *> is...*-i2.h
  // IWYU: PartiallySpecializedTpl<:0 *>::i is...*-i1.h
  PartiallySpecializedTpl<int*>::i = 1;
  // IWYU: TplWithMapping is...*-i2.h
  // IWYU: TplWithMapping::i is...*-i3.h
  TplWithMapping<int>::i = 1;
  // IWYU: PartiallySpecializedTplWithMapping<:0 *> is...*-i2.h
  // IWYU: PartiallySpecializedTplWithMapping<:0 *>::i is...*-i4.h
  PartiallySpecializedTplWithMapping<int*>::i = 1;
}

void TestTemplateArguments() {
  // IWYU: Tpl is...*-i2.h
  // IWYU: Tpl::fwd_decl_use_in_type is...*-i1.h
  // IWYU: IndirectClass needs a declaration
  (void)Tpl<IndirectClass>::fwd_decl_use_in_type;

  // IWYU: Tpl is...*-i2.h
  // IWYU: Tpl::full_use_in_type is...*-i1.h
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)Tpl<IndirectClass>::full_use_in_type;

  // IWYU: Tpl is...*-i2.h
  // IWYU: Tpl::full_use_in_init is...*-i1.h
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)Tpl<IndirectClass>::full_use_in_init;

  // IWYU: Tpl is...*-i2.h
  // IWYU: Tpl::fully_using_both is...*-i1.h
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectTemplate is...*indirect.h
  (void)Tpl<IndirectClass>::fully_using_both<IndirectTemplate<int>>;

  // IWYU: TplProvidingIC is...*-i1.h
  // IWYU: Tpl::full_use_in_type is...*-i1.h
  (void)TplProvidingIC::full_use_in_type;
  // IWYU: TplNonProvidingIC is...*-i2.h
  // IWYU: Tpl::full_use_in_type is...*-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)TplNonProvidingIC::full_use_in_type;
}

/**** IWYU_SUMMARY

tests/cxx/static_data_member.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/static_data_member-i1.h"
#include "tests/cxx/static_data_member-i2.h"
#include "tests/cxx/static_data_member-i3.h"
#include "tests/cxx/static_data_member-i4.h"

tests/cxx/static_data_member.cc should remove these lines:
- #include "tests/cxx/static_data_member-d1.h"  // lines XX-XX

The full include-list for tests/cxx/static_data_member.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate
#include "tests/cxx/static_data_member-i1.h"  // for PartiallySpecializedTpl<>::i, Tpl::full_use_in_init, Tpl::full_use_in_type, Tpl::fully_using_both, Tpl::fwd_decl_use_in_type, Tpl::i, Tpl<>::i, TplProvidingIC
#include "tests/cxx/static_data_member-i2.h"  // for PartiallySpecializedTpl, PartiallySpecializedTplWithMapping, Tpl, TplNonProvidingIC, TplWithMapping
#include "tests/cxx/static_data_member-i3.h"  // for TplWithMapping::i
#include "tests/cxx/static_data_member-i4.h"  // for PartiallySpecializedTplWithMapping<>::i

***** IWYU_SUMMARY */
