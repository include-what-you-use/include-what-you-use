//===--- guarded_include_spelling-i1.h - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_SUBDIR_GUARDED_INCLUDE_SPELLING_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_SUBDIR_GUARDED_INCLUDE_SPELLING_I1_H_

struct SpellingI1 {};

#define SPELLING_I1_INCLUDED
#include "guarded_include_spelling-i2.h"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_SUBDIR_GUARDED_INCLUDE_SPELLING_I1_H_
