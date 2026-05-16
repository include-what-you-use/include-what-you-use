//===--- explicit_instantiation-nested.h - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION_NESTED_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION_NESTED_H_

#include "tests/cxx/explicit_instantiation-template.h"

template <typename T>
void Host<T>::Fn() {
}

template <typename T>
void Host<T>::StaticFn() {
}

template <typename T>
int Host<T>::i;

template <typename T>
template <typename U>
U Host<T>::var_tpl;

template <typename T>
template <typename U>
char Host<T>::var_tpl<U*>;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION_NESTED_H_
