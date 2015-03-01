//===--- badinc-inl.h - test input file for iwyu --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_INL_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_INL_H_

#include <time.h>   // one way to get NULL
#include "tests/cxx/badinc-private.h"
#include "tests/cxx/badinc-private2.h"

typedef int (*InlH_FunctionPtr)(int);

InlH_FunctionPtr InlH_Function(class InlH_Class* c=NULL) {
  return NULL;
}

HPrivate_Enum InlH_PrivateFunction() { return HP1; }

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_INL_H_


/**** IWYU_SUMMARY

(tests/cxx/badinc-inl.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
