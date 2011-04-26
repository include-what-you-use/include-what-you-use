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

#include <set>
#include <string>
#include <vector>
#include "port.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/SourceManager.h"

namespace clang {
class HeaderSearch;
}

namespace include_what_you_use {

using std::set;
using std::string;

using std::vector;

class FullUseCache;
class IncludePicker;
class SourceManagerCharacterDataGetter;

// The following two routines are called to set up the global state --
// the first one right when main starts, and the second after the
// clang infrastructure is set up.  The rest of this file is
// accessors to the data structures set up by these two routines.

// Returns the index into argv past all the iwyu commandline flags.
int ParseIwyuCommandlineFlags(int argc, char** argv);

void InitGlobals(clang::SourceManager* source_manager,
                 clang::HeaderSearch* header_search);

// Can be called by tests -- doesn't need a SourceManager or
// argc/argv.  Note that GlobalSourceManager() and DefaultDataGetter()
// will assert-fail if you call this instead of InitGlobals().
void InitGlobalsAndFlagsForTesting();


// TODO(csilvers): put all of these in the 'globals' namespace?

// The verbose level indicates how chatty IWYU should be.  The bigger
// the chattier.
//   0: just print the full list of header files.
//   1: like 0, and also print new and deleted header files.
//   2: like 1, and also print 'canonical' error line for each type.
//   3: like 1, and also print *every* error line.
//   4: like 3, but with sorted error lines, and print all 'reason' comments.
//   5: like 4, and add tons of debug info.
//   6: like 5, and print every symbol that's needed by main-cu files.
//   7: like 6, and print pointers of AST nodes (and uninstantiated templates)
//  10: like 7, and add tons more debug info (for all non-system header files).
//  11: like 10, and add tons *more* debug info (for all header files).
struct CommandlineFlags {
  CommandlineFlags();                     // sets flags to default values
  int ParseArgv(int argc, char** argv);   // parses flags from argv
  static const char kUnspecified[];  // for -d, which takes an optional arg

  set<string> check_also;  // -c: globs to report iwyu violations for
  string howtodebug;       // -d: file to print gdb-invoking instruction for
  string cwd;              // -p: what directory was iwyu invoked from?
  int verbose;             // -v: how much information to emit as we parse
};

const CommandlineFlags& GlobalFlags();
// Used by tests as an easy way to simulate calling with different --flags.
CommandlineFlags* MutableGlobalFlagsForTesting();

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
