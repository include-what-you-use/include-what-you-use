//===--- default_tpl_arg-d2.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

// Use of this function template instantiated with the default argument doesn't
// require complete 'IndirectClass' type because this header '#include's it
// directly and hence provides.
template <typename T = IndirectClass>
void FnWithProvidedDefaultTplArg() {
  T t;
}

// Use of this function template instantiated with the default argument and
// called **without explicit ordinary argument** doesn't require complete
// 'IndirectClass' type because this header '#include's it directly and hence
// provides.
template <typename T = IndirectClass>
void FnWithProvidedDefaultTplArgAndDefaultCallArg1(T* = nullptr) {
  T t;
}

template <typename T = IndirectClass>
void FnWithProvidedDefaultTplArgAndDefaultCallArg2(T = T{}) {
  T t;
}

template <typename T = IndirectClass>
void FnWithProvidedDefaultTplArgAndDefaultCallArg3(T = {}) {
  T t;
}
