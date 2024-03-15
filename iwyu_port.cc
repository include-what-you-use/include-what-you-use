//===--- iwyu_port.cc - portability shims ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#if defined(_WIN32)

#include "Shlwapi.h"  // for PathMatchSpecA

bool GlobMatchesPath(const char *glob, const char *path) {
  return PathMatchSpecA(path, glob);
}

#else  // #if defined(_WIN32)

#include <fnmatch.h>

bool GlobMatchesPath(const char *glob, const char *path) {
  return fnmatch(glob, path, 0) == 0;
}

#endif  // #if defined(_WIN32)
