//===--- depopulated_h_file.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_DEPOPULATED_H_FILE_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_DEPOPULATED_H_FILE_H_

#include "tests/cxx/depopulated_h_file-i1.h"

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_DEPOPULATED_H_FILE_H_

/**** IWYU_SUMMARY

tests/cxx/depopulated_h_file.h should add these lines:

tests/cxx/depopulated_h_file.h should remove these lines:
- #include "tests/cxx/depopulated_h_file-i1.h"  // lines XX-XX

The full include-list for tests/cxx/depopulated_h_file.h:

***** IWYU_SUMMARY */
