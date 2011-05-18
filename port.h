//===--- port.h - OS/cpu specific stuff for include-what-you-use ----------===//
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

#include <assert.h>
#define CHECK_(x)  assert(x)

#if defined(_MSC_VER)

# define NOMINMAX
# include <windows.h>

# define getcwd _getcwd
# define snprintf _snprintf

// Get a windows-equivalent fnmatch()
# include "Shlwapi.h"  // For PathMatchSpec

# pragma comment(lib, "Shlwapi.lib")

#define FNM_PATHNAME (1<<0)

inline int fnmatch(const char *pattern, const char *string, int flags) {
  return !PathMatchSpec(string, pattern);
}

// FIXME: This undef is necessary to prevent conflicts between llvm
//        and Windows headers.  Eventually fnmatch functionality
//        should be wrapped inside llvm's PathV2 library.
# undef interface    // used in Shlwapi.h

#else  // #if defined(_MSC_VER)

#include <fnmatch.h>  // IWYU pragma: export

#endif  // #if defined(_MSC_VER)

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_PORT_H_
