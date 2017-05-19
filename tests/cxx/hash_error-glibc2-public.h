//===--- hash_error-glibc2-public.h - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test for auto-mapping based on #error messages in private headers.

#define IWYU_HASH_ERROR_GLIBC2_PUBLIC_INCLUDED
#include "tests/cxx/hash_error-glibc2-private.h"
#undef IWYU_HASH_ERROR_GLIBC2_PUBLIC_INCLUDED
