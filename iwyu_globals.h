//===--- iwyu_globals.h - global variables for include-what-you-use -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// The source manager is used in just too many places.

#ifndef INCLUDE_WHAT_YOU_USE_IWYU_GLOBALS_H_
#define INCLUDE_WHAT_YOU_USE_IWYU_GLOBALS_H_

#include <set>                          // for set
#include <string>                       // for string
#include <vector>                       // for vector

#include "clang/Basic/FileEntry.h"

namespace clang {
class CompilerInstance;
class SourceManager;
struct PrintingPolicy;

namespace driver {
class ToolChain;
}  // namespace driver
}  // namespace clang

namespace include_what_you_use {

using std::set;
using std::string;
using std::vector;

class FullUseCache;
class IncludePicker;
class SourceManagerCharacterDataGetter;
enum class RegexDialect;

// To set up the global state you need to parse options with OptionsParser when
// main starts and to call InitGlobals after the clang infrastructure is set up.
// The rest of this file is accessors to the data structures set up by these two
// routines.

class OptionsParser {
 public:
  OptionsParser(int argc, char** argv);
  ~OptionsParser();

  int clang_argc() const {
    return clang_argc_;
  }
  const char** clang_argv() const {
    return clang_argv_;
  }

 private:
  int clang_argc_;
  const char** clang_argv_;
};

void InitGlobals(clang::CompilerInstance& compiler,
                 const clang::driver::ToolChain& toolchain);

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
//   8: like 7, and print all mappings, built-in and dynamic.
//  10: like 8, and add tons more debug info (for all non-system header files).
//  11: like 10, and add tons *more* debug info (for all header files).
struct CommandlineFlags {
  enum PrefixHeaderIncludePolicy { kAdd, kKeep, kRemove };
  CommandlineFlags();                     // sets flags to default values
  int ParseArgv(int argc, char** argv);   // parses flags from argv
  bool HasDebugFlag(const char* flag) const;
  bool HasExperimentalFlag(const char* flag) const;

  set<string> check_also;  // -c: globs to report iwyu violations for
  set<string> keep;        // -k: globs to force-keep includes for
  bool transitive_includes_only;   // -t: don't add 'new' #includes to files
  int verbose;             // -v: how much information to emit as we parse
  vector<string> mapping_files; // -m: mapping files
  bool no_default_mappings;     // -n: no default mappings
  // Truncate output lines to this length. No short option.
  int max_line_length;
  // Policy regarding files included via -include option.  No short option.
  PrefixHeaderIncludePolicy prefix_header_include_policy;
  bool pch_in_code;   // Treat the first seen include as a PCH. No short option.
  bool no_comments;   // Disable 'why' comments. No short option.
  bool update_comments; // Force 'why' comments. No short option.
  bool comments_with_namespace; // Show namespace in 'why' comments.
  bool no_fwd_decls;  // Disable forward declarations.
  bool quoted_includes_first; // Place quoted includes first in sort order.
  bool cxx17ns; // -C: C++17 nested namespace syntax
  int exit_code_error;   // Exit with this code for iwyu violations.
  int exit_code_always;  // Always exit with this exit code.
  set<string> dbg_flags; // Debug flags.
  set<string> exp_flags;       // Experimental flags.
  RegexDialect regex_dialect;  // Dialect for regular expression processing.
};

const CommandlineFlags& GlobalFlags();
// Used by tests as an easy way to simulate calling with different --flags.
CommandlineFlags* MutableGlobalFlagsForTesting();

clang::SourceManager* GlobalSourceManager();

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
bool ShouldReportIWYUViolationsFor(clang::OptionalFileEntryRef file);

// For the commandline option --keep.
// Similar to AddGlobToReportIWYUViolationsFor.
void AddGlobToKeepIncludes(const string& glob);
bool ShouldKeepIncludeFor(clang::OptionalFileEntryRef file);

}  // namespace include_what_you_use

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_GLOBALS_H_
