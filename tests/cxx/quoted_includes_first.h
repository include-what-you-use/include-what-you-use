//===--- quoted_includes_first.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <list>                     // Unused C++ header to trigger diagnostics
#include <exception>                // C++ header
#include <errno.h>                  // C header
#include "subdir/indirect_subdir.h" // User header

inline int GetBaseError() {
  return ENOENT;
}

extern std::exception global_exception;
extern IndirectSubDirClass global_var;
inline IndirectSubDirClass header_defined_var;

/**** IWYU_SUMMARY

tests/cxx/quoted_includes_first.h should add these lines:

tests/cxx/quoted_includes_first.h should remove these lines:
- #include <list>  // lines XX-XX

The full include-list for tests/cxx/quoted_includes_first.h:
#include "subdir/indirect_subdir.h"  // for IndirectSubDirClass
#include <errno.h>  // for ENOENT
#include <exception>  // for exception

***** IWYU_SUMMARY */
