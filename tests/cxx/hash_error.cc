//===--- hash_error.cc - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests automatic mapping based on known #error directives in private headers.
//
// GNU libc has the following two conventions:
// #error "Never include <private> directly; use <public> instead."
// #error "Never use <private> directly; include <public> instead."
//
// The glib library has the following convention:
// #error "Only <public> can be included directly."
//
// Use angled includes to match typical style for these library includes.

#include <tests/cxx/hash_error-glibc1-public.h>
#include <tests/cxx/hash_error-glibc2-public.h>
#include <tests/cxx/hash_error-glib-public.h>
#include "tests/cxx/hash_error-nomap.h"  // unused

static int total = glibc1_symbol + glibc2_symbol + glib_symbol;


/**** IWYU_SUMMARY
tests/cxx/hash_error.cc should add these lines:

tests/cxx/hash_error.cc should remove these lines:
- #include "tests/cxx/hash_error-nomap.h"  // lines XX-XX

The full include-list for tests/cxx/hash_error.cc:
#include <tests/cxx/hash_error-glib-public.h>  // for glib_symbol
#include <tests/cxx/hash_error-glibc1-public.h>  // for glibc1_symbol
#include <tests/cxx/hash_error-glibc2-public.h>  // for glibc2_symbol

***** IWYU_SUMMARY */
