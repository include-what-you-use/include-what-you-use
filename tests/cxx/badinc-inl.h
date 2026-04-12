//===--- badinc-inl.h - test input file for iwyu --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_INL_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_INL_H_

#include <locale.h>   // the way to get NULL without size_t
#include "tests/cxx/badinc-private.h"
#include "tests/cxx/badinc-private2.h"

typedef int (*InlH_FunctionPtr)(int);

// IWYU: NULL is...*stddef.h
InlH_FunctionPtr InlH_Function(class InlH_Class* c=NULL) {
  // IWYU: NULL is...*stddef.h
  return NULL;
}

HPrivate_Enum InlH_PrivateFunction() { return HP1; }

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_INL_H_


/**** IWYU_SUMMARY

tests/cxx/badinc-inl.h should add these lines:
#include <stddef.h>

tests/cxx/badinc-inl.h should remove these lines:
- #include <locale.h>  // lines XX-XX

The full include-list for tests/cxx/badinc-inl.h:
#include <stddef.h>  // for NULL
#include "tests/cxx/badinc-private.h"  // for HPrivate_Enum
#include "tests/cxx/badinc-private2.h"

***** IWYU_SUMMARY */
