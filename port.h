//===--- iwyu_string_util.h - global variables for include-what-you-use ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Source file for architecture-specific logic.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_PORT_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_PORT_H_

#if defined(_MSC_VER)

# define NOMINMAX
# include <windows.h>
// FIXME: This undef is necessary to prevent conflicts between llvm
//        and Windows headers.  Eventually fnmatch functionality
//        should be wrapped inside llvm's PathV2 library.
# undef interface

# define getcwd _getcwd
# define snprintf _snprintf

// Get a windows-equivalent fnmatch()
# include "Shlwapi.h"  // For PathMatchSpec
# pragma comment(lib, "Shlwapi.lib")
# define fnmatch(pattern, filepath, flags)  (!PathMatchSpec(filepath, pattern))

#endif  // #if defined(_MSC_VER)

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_PORT_H_
