//===--- default_tpl_arg-d2.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/default_tpl_arg-d1.h"
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

template <int, int = 0, int>
// IWYU: AliasTpl5 is...*default_tpl_arg-i1.h
using AliasTpl5 = int;

// IWYU: ClassTpl2 is...*default_tpl_arg-i1.h
ClassTpl2<>& GetClassTpl2Ref();

/**** IWYU_SUMMARY

tests/cxx/default_tpl_arg-d2.h should add these lines:
#include "tests/cxx/default_tpl_arg-i1.h"

tests/cxx/default_tpl_arg-d2.h should remove these lines:
- #include "tests/cxx/default_tpl_arg-d1.h"  // lines XX-XX

The full include-list for tests/cxx/default_tpl_arg-d2.h:
#include "tests/cxx/default_tpl_arg-i1.h"  // for AliasTpl5, ClassTpl2
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
