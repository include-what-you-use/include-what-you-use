//===--- iwyu_verrs.h - debug output for include-what-you-use -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This module controls logging and verbosity levels for include-what-you-use.

#ifndef INCLUDE_WHAT_YOU_USE_IWYU_VERRS_H_
#define INCLUDE_WHAT_YOU_USE_IWYU_VERRS_H_

#include "clang/Basic/FileEntry.h"
#include "llvm/Support/raw_ostream.h"

namespace include_what_you_use {

void SetVerboseLevel(int level);
int GetVerboseLevel();

// Returns true if we should print a message at the given verbosity level.
inline bool ShouldPrint(int verbose_level) {
  return verbose_level <= GetVerboseLevel();
}

// Returns true if we should print information about a symbol in the
// given file, at the current verbosity level.  For instance, at most
// normal verbosities, we don't print information about symbols in
// system header files.
bool ShouldPrintSymbolFromFile(clang::OptionalFileEntryRef file);

// VERRS(n) << blah;
// prints blah to errs() if the verbose level is >= n.
#define VERRS(verbose_level) \
  if (!::include_what_you_use::ShouldPrint( \
          verbose_level)) ; else ::llvm::errs()

// Prints to errs() if the verbose level is at a high enough level to
// print symbols that occur in the given file.  This is only valid
// when used inside a class, such as IwyuAstConsumer, that defines a
// method named ShouldPrintSymbolFromFile().
#define ERRSYM(file_entry) \
  if (!ShouldPrintSymbolFromFile(file_entry)) ; else ::llvm::errs()

}  // namespace include_what_you_use

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_VERRS_H_
