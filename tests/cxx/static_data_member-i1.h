//===--- static_data_member-i1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/direct.h"
#include "tests/cxx/static_data_member-i2.h"

template <typename T>
int Tpl<T>::i;

// Test that an explicit specialization is still to be traversed (as opposed
// to implicit ones).
template <>
// IWYU: IndirectClass is...*indirect.h
int Tpl<char>::i = sizeof(IndirectClass);

template <typename T>
int PartiallySpecializedTpl<T*>::i;

template <typename T>
int TplWithMapping<T>::i;

template <typename T>
int PartiallySpecializedTplWithMapping<T*>::i;

template <typename T>
T* Tpl<T>::fwd_decl_use_in_type;

template <typename T>
T Tpl<T>::full_use_in_type;

template <typename T>
int Tpl<T>::full_use_in_init = (int)sizeof(T);

template <typename T>
template <typename U>
U Tpl<T>::fully_using_both = [] {
  T t;
  return U{};
}();

// IWYU: IndirectClass is...*indirect.h
using TplProvidingIC = Tpl<IndirectClass>;

/**** IWYU_SUMMARY

tests/cxx/static_data_member-i1.h should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/static_data_member-i1.h should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/static_data_member-i1.h:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/static_data_member-i2.h"  // for PartiallySpecializedTpl, PartiallySpecializedTplWithMapping, Tpl, TplWithMapping

***** IWYU_SUMMARY */
