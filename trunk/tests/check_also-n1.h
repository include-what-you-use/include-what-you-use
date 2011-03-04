//===--- check_also-n1.h - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the '--check_also' flag.
//
// This file is identical to check_also-d1.h, but has no iwyu summary,
// since 'n1.h' isn't part of the --see_also glob.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_CHECK_ALSO_D1_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_CHECK_ALSO_D1_H_

#include "check_also-i1.h"

int* unused = NULL;   // NULL comes from check_also-i1.h

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_CHECK_ALSO_D1_H_
