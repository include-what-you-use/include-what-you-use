//===--- direct_subdir.h - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file includes only another file in the subdir

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_DIRECT_SUBDIR_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_DIRECT_SUBDIR_H_

#include "indirect_subdir.h"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_DIRECT_SUBDIR_H_
