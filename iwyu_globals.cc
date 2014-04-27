//===--- iwyu_globals.cc - global variables for include-what-you-use ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_globals.h"

#include <stdio.h>                      // for printf
#include <stdlib.h>                     // for atoi, exit, getenv
#include <algorithm>                    // for sort, make_pair
#include <map>                          // for map
#include <set>                          // for set
#include <string>                       // for string, operator<, etc
#include <utility>                      // for make_pair, pair

#include "iwyu_cache.h"
#include "iwyu_include_picker.h"
#include "iwyu_getopt.h"
#include "iwyu_lexer_utils.h"
#include "iwyu_location_util.h"
#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"
#include "iwyu_version.h"
#include "port.h"  // for CHECK_, etc
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/Version.h"
#include "clang/Lex/HeaderSearch.h"

using clang::DirectoryEntry;
using std::make_pair;
using std::map;
using std::string;
using std::vector;

namespace include_what_you_use {

static CommandlineFlags* commandline_flags = NULL;
static clang::SourceManager* source_manager = NULL;
static IncludePicker* include_picker = NULL;
static const clang::LangOptions default_lang_options;
static const clang::PrintingPolicy default_print_policy(default_lang_options);
static SourceManagerCharacterDataGetter* data_getter = NULL;
static FullUseCache* function_calls_full_use_cache = NULL;
static FullUseCache* class_members_full_use_cache = NULL;
static int ParseIwyuCommandlineFlags(int argc, char** argv);
static int ParseInterceptedCommandlineFlags(int argc, char** argv);

static void PrintHelp(const char* extra_msg) {
  printf("USAGE: include-what-you-use [-Xiwyu --iwyu_opt]... <clang opts>"
         " <source file>\n"
         "Here are the <iwyu_opts> you can specify (e.g. -Xiwyu --verbose=3):\n"
         "   --check_also=<glob>: tells iwyu to print iwyu-violation info\n"
         "        for all files matching the given glob pattern (in addition\n"
         "        to the default of reporting for the input .cc file and its\n"
         "        associated .h files).  This flag may be specified multiple\n"
         "        times to specify multiple glob patterns.\n"
         "   --cwd=<dir>: tells iwyu what the current working directory is.\n"
         "   --howtodebug[=<filename>]: with no arg, prints instructions on\n"
         "        how to run iwyu under gdb for the input file, and exits.\n"
         "        With an arg, prints only when input file matches the arg.\n"
         "   --mapping_file=<filename>: gives iwyu a mapping file.\n"
         "   --no_default_mappings: do not add iwyu's default mappings.\n"
         "   --transitive_includes_only: do not suggest that a file add\n"
         "        foo.h unless foo.h is already visible in the file's\n"
         "        transitive includes.\n"
         "   --prefix_header_includes=<value>: tells iwyu what to do with\n"
         "        in-source includes and forward declarations involving\n"
         "        prefix headers.  Prefix header is a file included via\n"
         "        command-line option -include.  If prefix header makes\n"
         "        include or forward declaration obsolete, presence of such\n"
         "        include can be controlled with the following values\n"
         "          add:    new lines are added\n"
         "          keep:   new lines aren't added, existing are kept intact\n"
         "          remove: new lines aren't added, existing are removed\n"
         "        Default value is 'add'.\n"
         "   --pch_in_code: mark the first include in a translation unit as a\n"
         "        precompiled header.  Use --pch_in_code to prevent IWYU from\n"
         "        removing necessary PCH includes.  Though Clang forces PCHs\n"
         "        to be listed as prefix headers, the PCH-in-code pattern can\n"
         "        be used with GCC and is standard practice on MSVC\n"
         "        (e.g. stdafx.h).\n"
         "   --verbose=<level>: the higher the level, the more output.\n"
         "\n"
         "In addition to IWYU-specific options you can specify the following\n"
         "options without -Xiwyu prefix:\n"
         "   --help: prints this help and exits.\n"
         "   --version: prints version and exits.\n");
  if (extra_msg)
    printf("\n%s\n\n", extra_msg);
}

static void PrintVersion() {
  llvm::outs() << "include-what-you-use " << IWYU_VERSION_STRING;
  // IWYU_SVN_REVISION should be provided by build system.
  string iwyu_svn_revision = IWYU_SVN_REVISION;
  if (!iwyu_svn_revision.empty()) {
    llvm::outs() << " (" << iwyu_svn_revision << ")";
  }
  llvm::outs() << " based on " << clang::getClangFullVersion()
               << "\n";
}

OptionsParser::OptionsParser(int argc, char** argv) {
  // Separate out iwyu-specific, intercepted, and clang flags.  iwyu-specific
  // flags are "-Xiwyu <iwyu_flag>", intercepted flags are usual clang flags
  // like --version, --help, which we intercept to  provide custom handling.
  char** iwyu_argv = new char*[argc + 1];
  iwyu_argv[0] = argv[0];
  int iwyu_argc = 1;
  char** intercepted_argv = new char*[argc + 1];
  intercepted_argv[0] = argv[0];
  int intercepted_argc = 1;
  clang_argv_ = new const char*[argc + 1];
  clang_argv_[0] = argv[0];
  clang_argc_ = 1;
  for (int i = 1; i < argc; ++i) {
    if (i < argc - 1 && strcmp(argv[i], "-Xiwyu") == 0)
      iwyu_argv[iwyu_argc++] = argv[++i];   // the word after -Xiwyu
    else if (strcmp(argv[i], "--help") == 0)
      intercepted_argv[intercepted_argc++] = argv[i];     // intercept --help
    else if (strcmp(argv[i], "--version") == 0)
      intercepted_argv[intercepted_argc++] = argv[i];     // intercept --version
    else
      clang_argv_[clang_argc_++] = argv[i];
  }
  // argv should be NULL-terminated
  iwyu_argv[iwyu_argc] = NULL;
  intercepted_argv[intercepted_argc] = NULL;
  clang_argv_[clang_argc_] = NULL;

  ParseInterceptedCommandlineFlags(intercepted_argc, intercepted_argv);
  ParseIwyuCommandlineFlags(iwyu_argc, iwyu_argv);

  delete [] iwyu_argv;
  delete [] intercepted_argv;
}

OptionsParser::~OptionsParser() {
  delete [] clang_argv_;
}

CommandlineFlags::CommandlineFlags()
    : check_also(),
      howtodebug(CommandlineFlags::kUnspecified),
      cwd(""),
      transitive_includes_only(false),
      verbose(getenv("IWYU_VERBOSE") ? atoi(getenv("IWYU_VERBOSE")) : 1),
      no_default_mappings(false),
      prefix_header_include_policy(CommandlineFlags::kAdd),
      pch_in_code(false) {
}

int CommandlineFlags::ParseArgv(int argc, char** argv) {
  static const struct option longopts[] = {
    {"check_also", required_argument, NULL, 'c'},  // can be specified >once
    {"howtodebug", optional_argument, NULL, 'd'},
    {"cwd", required_argument, NULL, 'p'},
    {"transitive_includes_only", no_argument, NULL, 't'},
    {"verbose", required_argument, NULL, 'v'},
    {"mapping_file", required_argument, NULL, 'm'},
    {"no_default_mappings", no_argument, NULL, 'n'},
    {"prefix_header_includes", required_argument, NULL, 'x'},
    {"pch_in_code", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
  };
  static const char shortopts[] = "d::p:v:c:m:n";
  while (true) {
    switch (getopt_long(argc, argv, shortopts, longopts, NULL)) {
      case 'c': AddGlobToReportIWYUViolationsFor(optarg); break;
      case 'd': howtodebug = optarg ? optarg : ""; break;
      case 'p': cwd = optarg; break;
      case 't': transitive_includes_only = true; break;
      case 'v': verbose = atoi(optarg); break;
      case 'm': mapping_files.push_back(optarg); break;
      case 'n': no_default_mappings = true; break;
      case 'x':
        if (strcmp(optarg, "add") == 0) {
          prefix_header_include_policy = CommandlineFlags::kAdd;
        } else if (strcmp(optarg, "keep") == 0) {
          prefix_header_include_policy = CommandlineFlags::kKeep;
        } else if (strcmp(optarg, "remove") == 0) {
          prefix_header_include_policy = CommandlineFlags::kRemove;
        } else {
          PrintHelp("FATAL ERROR: unknown --prefix_header_includes value.");
          exit(1);
        }
        break;
      case 'h': pch_in_code = true; break;
      case -1: return optind;   // means 'no more input'
      default: PrintHelp("FATAL ERROR: unknown flag."); exit(1); break;
    }
  }
  return optind;  // unreachable
}

// Though option -v prints version too, it isn't intercepted because it also
// provides other functionality like printing clang invocation, header search
// paths.
// TODO(vsapsai): provide IWYU version in Driver::PrintVersion when version
// callbacks are supported (see FIXME in Driver::PrintVersion).
static int ParseInterceptedCommandlineFlags(int argc, char** argv) {
  static const struct option longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {0, 0, 0, 0}
  };
  static const char shortopts[] = "";
  while (true) {
    switch (getopt_long(argc, argv, shortopts, longopts, NULL)) {
      case 'h': PrintHelp(""); exit(0); break;
      case 'v': PrintVersion(); exit(0); break;
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

// Handles all iwyu-specific flags, like --verbose.  Returns the index into
// argv past all the iwyu commandline flags.
static int ParseIwyuCommandlineFlags(int argc, char** argv) {
  CHECK_(commandline_flags == NULL && "Only parse commandline flags once");
  commandline_flags = new CommandlineFlags;
  const int retval = commandline_flags->ParseArgv(argc, argv);
  SetVerboseLevel(commandline_flags->verbose);

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
    if (const DirectoryEntry* entry = it->getDir()) {
      const string path = CanonicalizeFilePath(entry->getName());
      search_path_map[path] = HeaderSearchPath::kSystemPath;
    }
  }
  for (clang::HeaderSearch::search_dir_iterator
           it = header_search->search_dir_begin();
       it != header_search->search_dir_end(); ++it) {
    if (const DirectoryEntry* entry = it->getDir()) {
      // search_dir_begin()/end() includes both system and user paths.
      // If it's a system path, it's already in the map, so everything
      // new is a user path.  The insert only 'takes' for new entries.
      const string path = CanonicalizeFilePath(entry->getName());
      search_path_map.insert(make_pair(path, HeaderSearchPath::kUserPath));
    }
  }
  return NormalizeHeaderSearchPaths(search_path_map);
}

void InitGlobals(clang::SourceManager* sm,
                 clang::HeaderSearch* header_search) {
  CHECK_(sm && "InitGlobals() needs a non-NULL SourceManager");
  source_manager = sm;
  data_getter = new SourceManagerCharacterDataGetter(*source_manager);
  vector<HeaderSearchPath> search_paths =
      ComputeHeaderSearchPaths(header_search);
  SetHeaderSearchPaths(search_paths);
  include_picker = new IncludePicker(GlobalFlags().no_default_mappings);
  function_calls_full_use_cache = new FullUseCache;
  class_members_full_use_cache = new FullUseCache;

  for (Each<HeaderSearchPath> it(&search_paths); !it.AtEnd(); ++it) {
    const char* path_type_name
        = (it->path_type == HeaderSearchPath::kSystemPath ? "system" : "user");
    VERRS(6) << "Search path: " << it->path << " (" << path_type_name << ")\n";
  }

  // Add mappings.
  for (Each<string> it(&GlobalFlags().mapping_files); !it.AtEnd(); ++it) {
    include_picker->AddMappingsFromFile(*it);
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
  commandline_flags->check_also.insert(CanonicalizeFilePath(glob));
}

bool ShouldReportIWYUViolationsFor(const clang::FileEntry* file) {
  const string filepath = GetFilePath(file);
  for (Each<string> it(&GlobalFlags().check_also); !it.AtEnd(); ++it)
    if (GlobMatchesPath(it->c_str(), filepath.c_str()))
      return true;
  return false;
}

void InitGlobalsAndFlagsForTesting() {
  CHECK_(commandline_flags == NULL && "Only parse commandline flags once");
  CHECK_(include_picker == NULL && "Only call InitGlobals[ForTesting] once");
  commandline_flags = new CommandlineFlags;
  source_manager = NULL;
  data_getter = NULL;
  include_picker = new IncludePicker(GlobalFlags().no_default_mappings);
  function_calls_full_use_cache = new FullUseCache;
  class_members_full_use_cache = new FullUseCache;

  // Use a reasonable default for the -I flags.
  map<string, HeaderSearchPath::Type> search_path_map;
  search_path_map["/usr/include"] = HeaderSearchPath::kSystemPath;
  search_path_map["/usr/include/c++/4.3"] = HeaderSearchPath::kSystemPath;
  search_path_map["/usr/include/c++/4.2"] = HeaderSearchPath::kSystemPath;
  search_path_map["."] = HeaderSearchPath::kUserPath;
  search_path_map["/usr/src/linux-headers-2.6.24-gg23/include"] = HeaderSearchPath::kSystemPath;

  SetHeaderSearchPaths(NormalizeHeaderSearchPaths(search_path_map));
}

}  // namespace include_what_you_use
