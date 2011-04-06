//===--- iwyu_string_util.h - global variables for include-what-you-use ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// The source manager is used in just too many places.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_GLOBALS_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_GLOBALS_H_

#include <string>
#include <vector>
#include "port.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/SourceManager.h"

namespace clang {
class HeaderSearch;
}

namespace include_what_you_use {

using std::string;

using std::vector;

class FullUseCache;
class IncludePicker;
class SourceManagerCharacterDataGetter;

void InitGlobals(clang::SourceManager* source_manager,
                 clang::HeaderSearch* header_search);

// Can be called by tests -- doesn't need a SourceManager.  Note that
// GlobalSourceManager() and DefaultDataGetter() will assert-fail if
// you call this instead of InitGlobals().
void InitGlobalsForTesting();

// TODO(csilvers): put all of these in the 'globals' namespace?

clang::SourceManager* GlobalSourceManager();

// The directories to look for #includes in, including from -I, -isystem, etc.
const vector<string>& GlobalSearchPaths();

const IncludePicker& GlobalIncludePicker();
IncludePicker* MutableGlobalIncludePicker();   // only use at great need!

const clang::PrintingPolicy& DefaultPrintPolicy();

const SourceManagerCharacterDataGetter& DefaultDataGetter();

// These caches record what types and decls we reported when
// instantiating a particular decl.  That avoids extra work if we see
// the same decl again -- we can replay those reports, just from a new
// caller_loc.
FullUseCache* FunctionCallsFullUseCache();
FullUseCache* ClassMembersFullUseCache();


// These files are based on the commandline (--check_also flag plus argv).
// They are specified as glob file-patterns (which behave just as they
// do in the shell).  TODO(csilvers): use a prefix instead? allow '...'?
void AddGlobToReportIWYUViolationsFor(const string& glob);
bool ShouldReportIWYUViolationsFor(const clang::FileEntry* file);

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_GLOBALS_H_
