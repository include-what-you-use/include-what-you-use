//===--- default_tpl_arg-i1.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

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
