//===--- default_tpl_arg-i1.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_DEFAULT_TPL_ARG_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_DEFAULT_TPL_ARG_I1_H_

template <typename T>
struct UninstantiatedTpl {
  T t;
};

class IndirectClass;

// Use of this function template instantiated with the default argument requires
// complete 'IndirectClass' type info because this header doesn't provide it.
template <typename T = IndirectClass>
void FnWithNonProvidedDefaultTplArg() {
  T t;
}

// Use of this function template instantiated with the default argument and
// called **without explicit ordinary argument** requires complete
// 'IndirectClass' type info because this header doesn't provide it.
template <typename T = IndirectClass>
void FnWithNonProvidedDefaultTplArgAndDefaultCallArg(T* = nullptr) {
  T t;
}

using NonProvidingAlias = IndirectClass;

template <typename, typename = int>
class ClassTpl;

template <typename, typename = int>
class ClassTplWithDefinition;

template <typename, typename = int>
extern int VarTpl;

template <typename, typename = int>
using AliasTpl1 = int;

template <typename, typename = int>
void FnTpl();

template <int, int = 0>
using AliasTpl2 = int;

template <int>
class SomeTpl;

template <template <int> typename, template <int> typename = SomeTpl>
using AliasTpl3 = int;

template <typename, int = 0>
using AliasTpl4 = int;

template <int, int, int = 0>
using AliasTpl5 = int;

template <int, int>
using AliasTpl6 = int;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_DEFAULT_TPL_ARG_I1_H_
