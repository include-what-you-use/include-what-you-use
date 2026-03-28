//===--- 1942.h - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "f-def_arg.h"

template <typename>
void f(int);

/**** IWYU_SUMMARY

(tests/bugs/1942/1942.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
