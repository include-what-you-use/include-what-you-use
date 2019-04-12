//===--- export_near.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file is meant to be a generic exporting header.  Similarly to
// direct_near.h, it includes indirect.h.  However, unlike that file, this one
// exports indirect.h, and therefore is a valid file to include to access
// IndirectClass.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPORT_NEAR_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPORT_NEAR_H_

#include "indirect.h" // IWYU pragma: export

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPORT_NEAR_H_
