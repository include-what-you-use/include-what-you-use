//===--- iwyu_string_util.cpp - global variables for include-what-you-use -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_globals.h"

#ifndef _MSC_VER      // _MSC_VER gets its own fnmatch from ./port.h
#include <fnmatch.h>
#endif
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>

#include "iwyu_cache.h"
#include "iwyu_include_picker.h"
#include "iwyu_lexer_utils.h"
#include "iwyu_location_util.h"
#include "iwyu_output.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "port.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/HeaderSearch.h"

using clang::DirectoryEntry;
using clang::DirectoryLookup;
using std::make_pair;
using std::map;
using std::set;
using std::string;
using std::vector;

namespace include_what_you_use {

static CommandlineFlags* commandline_flags = NULL;
static clang::SourceManager* source_manager = NULL;
static vector<HeaderSearchPath>* search_paths = NULL;
static IncludePicker* include_picker = NULL;
static const clang::LangOptions default_lang_options;
static const clang::PrintingPolicy default_print_policy(default_lang_options);
static SourceManagerCharacterDataGetter* data_getter = NULL;
static FullUseCache* function_calls_full_use_cache = NULL;
static FullUseCache* class_members_full_use_cache = NULL;


static void PrintHelp(const char* extra_msg) {
  printf("USAGE: iwyu [-Xiwyu --iwyu_opt]... <clang opts> <source file>\n"
         "Here are the <opts> you can specify:\n"
         "   --check_also=<glob>: tells iwyu to print iwyu-violation info\n"
         "        for all files matching the given glob pattern (in addition\n"
         "        to the default of reporting for the input .cc file and its\n"
         "        associated .h files).  This flag may be specified multiple\n"
         "        times to specify multiple glob patterns.\n"
         "   --cwd=<dir>: tells iwyu what the current working directory is.\n"
         "   --help: prints this help and exits.\n"
         "   --howtodebug[=<filename>]: with no arg, prints instructions on\n"
         "        how to run iwyu under gdb for the input file, and exits.\n"
         "        With an arg, prints only when input file matches the arg.\n"
         "   --transitive_includes_only: do not suggest that a file add\n"
         "        foo.h unless foo.h is already visible in the file's\n"
         "        transitive includes.\n"
         "   --verbose=<level>: the higher the level, the more output.\n");
  if (extra_msg)
    printf("\n%s\n\n", extra_msg);
}

CommandlineFlags::CommandlineFlags()
    : check_also(),
      howtodebug(CommandlineFlags::kUnspecified),
      cwd(""),
      transitive_includes_only(false),
      verbose(getenv("IWYU_VERBOSE") ? atoi(getenv("IWYU_VERBOSE")) : 1) {
}

int CommandlineFlags::ParseArgv(int argc, char** argv) {
  static const struct option longopts[] = {
    {"check_also", required_argument, NULL, 'c'},  // can be specified >once
    {"howtodebug", optional_argument, NULL, 'd'},
    {"help", no_argument, NULL, 'h'},
    {"cwd", required_argument, NULL, 'p'},
    {"transitive_includes_only", no_argument, NULL, 't'},
    {"verbose", required_argument, NULL, 'v'},
    {0, 0, 0, 0}
  };
  static const char shortopts[] = "d::p:v:c:";
  int option_index;
  while (true) {
    switch (getopt_long(argc, argv, shortopts, longopts, &option_index)) {
      case 'c': AddGlobToReportIWYUViolationsFor(optarg); break;
      case 'd': howtodebug = optarg ? optarg : ""; break;
      case 'h': PrintHelp(""); exit(0); break;
      case 'p': cwd = optarg; break;
      case 't': transitive_includes_only = true; break;
      case 'v': verbose = atoi(optarg); break;
      case -1: return optind;   // means 'no more input'
      default: PrintHelp("FATAL ERROR: unknown flag."); exit(1); break;
    }
  }
  return optind;  // unreachable
}

// The default value for the --howtodebug flag.  Indicates that the
// flag isn't present.  It's a special, reserved value, and a user
// isn't expected to type it directly.
const char CommandlineFlags::kUnspecified[] = "<flag-unspecified>";

// Handles all iwyu-specific flags, like --verbose.
int ParseIwyuCommandlineFlags(int argc, char** argv) {
  CHECK_(commandline_flags == NULL && "Only parse commandline flags once");
  commandline_flags = new CommandlineFlags;
  const int retval = commandline_flags->ParseArgv(argc, argv);

if (!commandline_flags->cwd.empty()) {
     printf("-p/--cwd not yet implemented\n");
     exit(1);
  }
  if (commandline_flags->howtodebug != CommandlineFlags::kUnspecified) {
     printf("-d/--howtodebug not yet implemented\n");
     exit(1);
  }

  VERRS(4) << "Setting verbose-level to " << commandline_flags->verbose << "\n";

  return retval;
}

// Make sure we put longer search-paths first, so iwyu will map
// /usr/include/c++/4.4/foo to <foo> rather than <c++/4.4/foo>.
static bool SortByDescendingLength(const HeaderSearchPath& left,
                                   const HeaderSearchPath& right) {
  return left.path.length() > right.path.length();
}

// Sorts them by descending length, does other kinds of cleanup.
static vector<HeaderSearchPath> NormalizeHeaderSearchPaths(
    const map<string, HeaderSearchPath::Type>& include_dirs_map) {
  vector<HeaderSearchPath> include_dirs;
  for (Each<string, HeaderSearchPath::Type>
           it(&include_dirs_map); !it.AtEnd(); ++it) {
    include_dirs.push_back(HeaderSearchPath(it->first, it->second));
  }


  sort(include_dirs.begin(), include_dirs.end(), &SortByDescendingLength);
  return include_dirs;
}

// Asks clang what the search-paths are for include files, normalizes
// them, and returns them in a vector.
static vector<HeaderSearchPath> ComputeHeaderSearchPaths(
    clang::HeaderSearch* header_search) {
  map<string, HeaderSearchPath::Type> search_path_map;
  for (clang::HeaderSearch::search_dir_iterator
           it = header_search->system_dir_begin();
       it != header_search->system_dir_end(); ++it) {
    if (const DirectoryEntry* entry = it->getDir())
      search_path_map[entry->getName()] = HeaderSearchPath::kSystemPath;
  }
  for (clang::HeaderSearch::search_dir_iterator
           it = header_search->search_dir_begin();
       it != header_search->search_dir_end(); ++it) {
    if (const DirectoryEntry* entry = it->getDir()) {
      // search_dir_begin()/end() includes both system and user paths.
      // If it's a system path, it's already in the map, so everything
      // new is a user path.  The insert only 'takes' for new entries.
      search_path_map.insert(make_pair(entry->getName(),
                                       HeaderSearchPath::kUserPath));
    }
  }
  return NormalizeHeaderSearchPaths(search_path_map);
}

void InitGlobals(clang::SourceManager* sm, clang::HeaderSearch* header_search) {
  CHECK_(sm && "InitGlobals() needs a non-NULL SourceManager");
  source_manager = sm;
  data_getter = new SourceManagerCharacterDataGetter(*source_manager);
  search_paths = new vector<HeaderSearchPath>(
      ComputeHeaderSearchPaths(header_search));
  include_picker = new IncludePicker;
  function_calls_full_use_cache = new FullUseCache;
  class_members_full_use_cache = new FullUseCache;

  for (Each<HeaderSearchPath> it(search_paths); !it.AtEnd(); ++it) {
    const char* path_type_name
        = (it->path_type == HeaderSearchPath::kSystemPath ? "system" : "user");
    VERRS(6) << "Search path: " << it->path << " (" << path_type_name << ")\n";
  }
}

const CommandlineFlags& GlobalFlags() {
  CHECK_(commandline_flags && "Call ParseIwyuCommandlineFlags() before this");
  return *commandline_flags;
}

CommandlineFlags* MutableGlobalFlagsForTesting() {
  CHECK_(commandline_flags && "Call ParseIwyuCommandlineFlags() before this");
  return commandline_flags;
}

clang::SourceManager* GlobalSourceManager() {
  CHECK_(source_manager && "Must call InitGlobals() before calling this");
  return source_manager;
}

const vector<HeaderSearchPath>& GlobalHeaderSearchPaths() {
  assert(search_paths && "Must call InitGlobals() before calling this");
  return *search_paths;
}

const IncludePicker& GlobalIncludePicker() {
  CHECK_(include_picker && "Must call InitGlobals() before calling this");
  return *include_picker;
}

IncludePicker* MutableGlobalIncludePicker() {
  CHECK_(include_picker && "Must call InitGlobals() before calling this");
  return include_picker;
}

const clang::PrintingPolicy& DefaultPrintPolicy() {
  return default_print_policy;
}

const SourceManagerCharacterDataGetter& DefaultDataGetter() {
  CHECK_(data_getter && "Must call InitGlobals() before calling this");
  return *data_getter;
}

FullUseCache* FunctionCallsFullUseCache() {
  return function_calls_full_use_cache;
}

FullUseCache* ClassMembersFullUseCache() {
  return class_members_full_use_cache;
}

void AddGlobToReportIWYUViolationsFor(const string& glob) {
  CHECK_(commandline_flags && "Call ParseIwyuCommandlineFlags() before this");
  commandline_flags->check_also.insert(glob);
}

bool ShouldReportIWYUViolationsFor(const clang::FileEntry* file) {
  const string filepath = GetFilePath(file);
  for (Each<string> it(&GlobalFlags().check_also); !it.AtEnd(); ++it)
    if (fnmatch(it->c_str(), filepath.c_str(), FNM_PATHNAME) == 0)
      return true;
  return false;
}

void InitGlobalsAndFlagsForTesting() {
  CHECK_(commandline_flags == NULL && "Only parse commandline flags once");
  CHECK_(include_picker == NULL && "Only call InitGlobals[ForTesting] once");
  commandline_flags = new CommandlineFlags;
  source_manager = NULL;
  data_getter = NULL;
  include_picker = new IncludePicker;
  function_calls_full_use_cache = new FullUseCache;
  class_members_full_use_cache = new FullUseCache;

  // Use a reasonable default for the -I flags.
  map<string, HeaderSearchPath::Type> search_path_map;
  search_path_map["/usr/include"] = HeaderSearchPath::kSystemPath;
  search_path_map["/usr/include/c++/4.3"] = HeaderSearchPath::kSystemPath;
  search_path_map["/usr/include/c++/4.2"] = HeaderSearchPath::kSystemPath;
  search_path_map["."] = HeaderSearchPath::kUserPath;
  search_path_map["/usr/src/linux-headers-2.6.24-gg23/include"] = HeaderSearchPath::kSystemPath;

  search_paths = new vector<HeaderSearchPath>(
      NormalizeHeaderSearchPaths(search_path_map));
}

}  // namespace include_what_you_use
