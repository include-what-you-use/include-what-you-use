//===--- quoted_includes_first.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --pch_in_code -Xiwyu --quoted_includes_first -I .

// Tests that IWYU will respect the --quoted_includes_first option.

#include "tests/cxx/pch.h"          // Precompiled header
#include <list>                     // Unused C++ header to trigger diagnostics
#include <exception>                // C++ header
#include <errno.h>                  // C header
#include "subdir/indirect_subdir.h" // User header
#include "quoted_includes_first.h"  // Associated header

static int global_err = EACCES;
std::exception global_exception;
IndirectSubDirClass global_var;

/**** IWYU_SUMMARY

tests/cxx/quoted_includes_first.cc should add these lines:

tests/cxx/quoted_includes_first.cc should remove these lines:
- #include <list>  // lines XX-XX

The full include-list for tests/cxx/quoted_includes_first.cc:
#include "tests/cxx/pch.h"
#include "quoted_includes_first.h"
#include "subdir/indirect_subdir.h"  // for IndirectSubDirClass
#include <errno.h>  // for EACCES
#include <exception>  // for exception

***** IWYU_SUMMARY */
