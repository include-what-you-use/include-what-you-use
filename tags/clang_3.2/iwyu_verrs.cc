//===--- iwyu_verrs.cc - debug output for include-what-you-use ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_verrs.h"

#include "iwyu_globals.h"
#include "iwyu_location_util.h"
#include "iwyu_path_util.h"

namespace include_what_you_use {

using clang::FileEntry;

namespace {
int verbose_level = 1;
}  // namespace

void SetVerboseLevel(int level) {
  verbose_level = level;
}

int GetVerboseLevel() {
  return verbose_level;
}

bool ShouldPrintSymbolFromFile(const FileEntry* file) {
  if (GetVerboseLevel() < 5) {
    return false;
  } else if (GetVerboseLevel() < 10) {
    return ShouldReportIWYUViolationsFor(file);
  } else if (GetVerboseLevel() < 11) {
    return !IsSystemIncludeFile(GetFilePath(file));
  } else {
    return true;
  }
}

}  // namespace include_what_you_use
