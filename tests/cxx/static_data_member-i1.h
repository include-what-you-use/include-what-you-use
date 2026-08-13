//===--- static_data_member-i1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"
#include "tests/cxx/static_data_member-i2.h"

template <typename T>
int Tpl<T>::i;

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

using TplProvidingIC = Tpl<IndirectClass>;
