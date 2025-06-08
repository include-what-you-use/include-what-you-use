//===--- iwyu_globals.cc - global variables for include-what-you-use ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_globals.h"

#include <algorithm>                    // for sort, make_pair
#include <climits>
#include <cstdio>                       // for printf
#include <cstdlib>                      // for atoi, exit, getenv
#include <cstring>
#include <map>                          // for map
#include <set>                          // for set
#include <string>                       // for string, operator<, etc
#include <utility>                      // for make_pair, pair

#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/DirectoryEntry.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/Version.h"
#include "clang/Driver/ToolChain.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/DirectoryLookup.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "iwyu_cache.h"
#include "iwyu_getopt.h"
#include "iwyu_include_picker.h"
#include "iwyu_lexer_utils.h"
#include "iwyu_location_util.h"
#include "iwyu_path_util.h"
#include "iwyu_port.h"  // for CHECK_, etc
#include "iwyu_regex.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"
#include "iwyu_version.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/ArgList.h"

using clang::CompilerInstance;
using clang::HeaderSearch;
using clang::LangOptions;
using clang::OptionalDirectoryEntryRef;
using clang::OptionalFileEntryRef;
using clang::PrintingPolicy;
using clang::SourceManager;
using clang::driver::ToolChain;
using clang::getClangFullVersion;
using std::make_pair;
using std::map;
using std::string;
using std::vector;

namespace include_what_you_use {

static CommandlineFlags* commandline_flags = nullptr;
static SourceManager* source_manager = nullptr;
static IncludePicker* include_picker = nullptr;
static const LangOptions default_lang_options;
static const PrintingPolicy default_print_policy(default_lang_options);
static SourceManagerCharacterDataGetter* data_getter = nullptr;
static FullUseCache* function_calls_full_use_cache = nullptr;
static FullUseCache* class_members_full_use_cache = nullptr;
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
         "   --keep=<glob>: tells iwyu to always keep these includes.\n"
         "        This flag may be specified multiple times to specify\n"
         "        multiple glob patterns.\n"
         "   --mapping_file=<filename>: gives iwyu a mapping file.\n"
         "   --no_internal_mappings: do not add iwyu's internal mappings.\n"
         "   --export_mappings=<dirpath>: writes out all internal mappings.\n"
         "   --pch_in_code: mark the first include in a translation unit as a\n"
         "        precompiled header.  Use --pch_in_code to prevent IWYU from\n"
         "        removing necessary PCH includes.  Though Clang forces PCHs\n"
         "        to be listed as prefix headers, the PCH-in-code pattern can\n"
         "        be used with GCC and is standard practice on MSVC\n"
         "        (e.g. stdafx.h).\n"
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
         "   --transitive_includes_only: do not suggest that a file add\n"
         "        foo.h unless foo.h is already visible in the file's\n"
         "        transitive includes.\n"
         "   --max_line_length: maximum line length for includes.\n"
         "        Note that this only affects comments and alignment thereof,\n"
         "        the maximum line length can still be exceeded with long\n"
         "        file names (default: 80).\n"
         "   --comment_style=<level> set verbosity of 'why' comments to one\n"
         "        of the following values:\n"
         "          none:  do not add 'why' comments\n"
         "          short: 'why' comments do not include namespaces\n"
         "          long:  'why' comments include namespaces\n"
         "        Default value is 'short'.\n"
         "   --no_comments: do not add 'why' comments.\n"
         "   --update_comments: update and insert 'why' comments, even if no\n"
         "        #include lines need to be added or removed.\n"
         "   --no_fwd_decls: do not use forward declarations.\n"
         "   --verbose=<level>: the higher the level, the more output.\n"
         "   --quoted_includes_first: when sorting includes, place quoted\n"
         "        ones first.\n"
         "   --cxx17ns: use C++17 nested namespaces when suggesting additions\n"
         "        of forward declarations.\n"
         "   --error[=N]: exit with N (default: 1) for iwyu violations\n"
         "   --error_always[=N]: always exit with N (default: 1) (for use\n"
         "        with 'make -k')\n"
         "   --debug=flag[,flag...]: debug flags (undocumented)\n"
         "   --regex=<dialect>: use specified regex dialect in IWYU:\n"
         "          llvm:       fast and simple (default)\n"
         "          ecmascript: slower, but more feature-complete\n"
         "   --experimental=flag[,flag...]: enable experimental features\n"
         "          clang_mappings: use Clang canonical standard library\n"
         "                          mappings instead of internal mappings\n"
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
  // IWYU_GIT_REV should be provided by build system.
  string iwyu_rev = IWYU_GIT_REV;
  if (!iwyu_rev.empty()) {
    llvm::outs() << " (git:" << iwyu_rev << ")";
  }
  llvm::outs() << " based on " << getClangFullVersion() << "\n";
}

static bool ParseIntegerOptarg(const char* optarg, int* res) {
  char* endptr = nullptr;
  long val = strtol(optarg, &endptr, 10);
  if (!endptr || endptr == optarg)
    return false;

  if (*endptr != '\0')
    return false;

  if (val > INT_MAX || val < INT_MIN)
    return false;

  *res = (int)val;
  return true;
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
  // argv should be nullptr-terminated
  iwyu_argv[iwyu_argc] = nullptr;
  intercepted_argv[intercepted_argc] = nullptr;
  clang_argv_[clang_argc_] = nullptr;

  ParseInterceptedCommandlineFlags(intercepted_argc, intercepted_argv);
  ParseIwyuCommandlineFlags(iwyu_argc, iwyu_argv);

  delete [] iwyu_argv;
  delete [] intercepted_argv;
}

OptionsParser::~OptionsParser() {
  delete [] clang_argv_;
}

CommandlineFlags::CommandlineFlags()
    : transitive_includes_only(false),
      verbose(getenv("IWYU_VERBOSE") ? atoi(getenv("IWYU_VERBOSE")) : 1),
      no_internal_mappings(false),
      max_line_length(80),
      prefix_header_include_policy(CommandlineFlags::kAdd),
      pch_in_code(false),
      no_comments(false),
      update_comments(false),
      comments_with_namespace(false),
      no_fwd_decls(false),
      quoted_includes_first(false),
      cxx17ns(false),
      exit_code_error(EXIT_SUCCESS),
      exit_code_always(EXIT_SUCCESS),
      regex_dialect(RegexDialect::LLVM) {
  // Always keep Qt .moc includes; its moc compiler does its own IWYU analysis.
  keep.emplace("*.moc");
}

int CommandlineFlags::ParseArgv(int argc, char** argv) {
  static const struct option longopts[] = {
    {"check_also", required_argument, nullptr, 'c'},  // can be specified >once
    {"keep", required_argument, nullptr, 'k'},  // can be specified >once
    {"transitive_includes_only", no_argument, nullptr, 't'},
    {"verbose", required_argument, nullptr, 'v'},
    {"mapping_file", required_argument, nullptr, 'm'},
    {"no_internal_mappings", no_argument, nullptr, 'n'},
    {"no_default_mappings", no_argument, nullptr, 'n'},  // deprecated
    {"prefix_header_includes", required_argument, nullptr, 'x'},
    {"pch_in_code", no_argument, nullptr, 'h'},
    {"max_line_length", required_argument, nullptr, 'l'},
    {"comment_style", required_argument, nullptr, 'i'},
    {"no_comments", no_argument, nullptr, 'o'},
    {"update_comments", no_argument, nullptr, 'u'},
    {"no_fwd_decls", no_argument, nullptr, 'f'},
    {"quoted_includes_first", no_argument, nullptr, 'q' },
    {"cxx17ns", no_argument, nullptr, 'C'},
    {"error", optional_argument, nullptr, 'e'},
    {"error_always", optional_argument, nullptr, 'a'},
    {"debug", required_argument, nullptr, 'd'},
    {"regex", required_argument, nullptr, 'r'},
    {"experimental", required_argument, nullptr, 'p'},
    {"export_mappings", required_argument, nullptr, 'E'},
    {nullptr, 0, nullptr, 0}
  };
  static const char shortopts[] = "v:c:m:d:nr";
  while (true) {
    switch (getopt_long(argc, argv, shortopts, longopts, nullptr)) {
      case 'c': AddGlobToReportIWYUViolationsFor(optarg); break;
      case 'k': AddGlobToKeepIncludes(optarg); break;
      case 't': transitive_includes_only = true; break;
      case 'v': verbose = atoi(optarg); break;
      case 'm': mapping_files.push_back(optarg); break;
      case 'n':
        no_internal_mappings = true;
        break;
      case 'o': no_comments = true; break;
      case 'u': update_comments = true; break;
      case 'i':
        if (strcmp(optarg, "none") == 0) {
          no_comments = true;
        } else if (strcmp(optarg, "short") == 0) {
          comments_with_namespace = false;
        } else if (strcmp(optarg, "long") == 0) {
          comments_with_namespace = true;
        } else {
          PrintHelp("FATAL ERROR: unknown comment style.");
          exit(EXIT_FAILURE);
        }
        break;
      case 'f': no_fwd_decls = true; break;
      case 'x':
        if (strcmp(optarg, "add") == 0) {
          prefix_header_include_policy = CommandlineFlags::kAdd;
        } else if (strcmp(optarg, "keep") == 0) {
          prefix_header_include_policy = CommandlineFlags::kKeep;
        } else if (strcmp(optarg, "remove") == 0) {
          prefix_header_include_policy = CommandlineFlags::kRemove;
        } else {
          PrintHelp("FATAL ERROR: unknown --prefix_header_includes value.");
          exit(EXIT_FAILURE);
        }
        break;
      case 'h': pch_in_code = true; break;
      case 'l':
        max_line_length = atoi(optarg);
        CHECK_((max_line_length >= 0) && "Max line length must be positive");
        break;
      case 'q': quoted_includes_first = true; break;
      case 'C': cxx17ns = true; break;
      case 'e':
        if (!optarg) {
          exit_code_error = EXIT_FAILURE;
        } else if (!ParseIntegerOptarg(optarg, &exit_code_error)) {
          PrintHelp("FATAL ERROR: --error argument must be valid integer.");
          exit(EXIT_FAILURE);
        }
        break;
      case 'a':
        if (!optarg) {
          exit_code_always = EXIT_FAILURE;
        } else if (!ParseIntegerOptarg(optarg, &exit_code_always)) {
          PrintHelp(
              "FATAL ERROR: --error_always argument must be valid "
              "integer.");
          exit(EXIT_FAILURE);
        }
        break;
      case 'd': {
        // Split argument on comma and save in global, ignoring empty elements.
        vector<string> flags = Split(optarg, ",", 0);
        dbg_flags.insert(flags.begin(),
                         std::remove(flags.begin(), flags.end(), string()));
        // Print all effective flags for traceability.
        for (const string& f : dbg_flags) {
          llvm::errs() << "Debug flag enabled: '" << f << "'\n";
        }
        break;
      }
      case 'r':
        if (!ParseRegexDialect(optarg, &regex_dialect)) {
          PrintHelp("FATAL ERROR: unsupported regex dialect.");
          exit(EXIT_FAILURE);
        }
        break;
      case 'p': {
        // Split argument on comma and save in global, ignoring empty elements.
        vector<string> flags = Split(optarg, ",", 0);
        exp_flags.insert(flags.begin(),
                         std::remove(flags.begin(), flags.end(), string()));
        // Print all effective flags for traceability.
        for (const string& f : exp_flags) {
          llvm::errs() << "Experimental flag enabled: '" << f << "'\n";
        }
        break;
      }
      case 'E': {
        // Handle --export_mappings immediately. We already depend on
        // iwyu_include_picker here, and we want to run the export before the
        // Clang/IWYU driver starts making demands for required inputs, etc.
        string output_dirpath(optarg);
        ExportInternalMappings(output_dirpath);
        exit(EXIT_SUCCESS);
        break;
      }
      case -1:
        return optind;  // means 'no more input'
      default:
        PrintHelp("FATAL ERROR: unknown flag.");
        exit(EXIT_FAILURE);
        break;
    }
  }

  CHECK_UNREACHABLE_("All switches should be handled above");
}

bool CommandlineFlags::HasDebugFlag(const char* flag) const {
  return dbg_flags.find(string(flag)) != dbg_flags.end();
}

bool CommandlineFlags::HasExperimentalFlag(const char* flag) const {
  return exp_flags.find(string(flag)) != exp_flags.end();
}

// Though option -v prints version too, it isn't intercepted because it also
// provides other functionality like printing clang invocation, header search
// paths.
// TODO(vsapsai): provide IWYU version in Driver::PrintVersion when version
// callbacks are supported (see FIXME in Driver::PrintVersion).
static int ParseInterceptedCommandlineFlags(int argc, char** argv) {
  static const struct option longopts[] = {
    {"help", no_argument, nullptr, 'h'},
    {"version", no_argument, nullptr, 'v'},
    {nullptr, 0, nullptr, 0}
  };
  static const char shortopts[] = "";
  while (true) {
    switch (getopt_long(argc, argv, shortopts, longopts, nullptr)) {
      case 'h': PrintHelp(""); exit(EXIT_SUCCESS); break;
      case 'v': PrintVersion(); exit(EXIT_SUCCESS); break;
      case -1:
        return optind;  // means 'no more input'
      default:
        PrintHelp("FATAL ERROR: unknown flag.");
        exit(EXIT_FAILURE);
        break;
    }
  }
  return optind;  // unreachable
}

// Handles all iwyu-specific flags, like --verbose.  Returns the index into
// argv past all the iwyu commandline flags.
static int ParseIwyuCommandlineFlags(int argc, char** argv) {
  CHECK_(commandline_flags == nullptr && "Only parse commandline flags once");
  commandline_flags = new CommandlineFlags;
  const int retval = commandline_flags->ParseArgv(argc, argv);
  SetVerboseLevel(commandline_flags->verbose);

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
  for (const auto& entry : include_dirs_map) {
    include_dirs.push_back(HeaderSearchPath(entry.first, entry.second));
  }

  sort(include_dirs.begin(), include_dirs.end(), &SortByDescendingLength);
  return include_dirs;
}

// Asks clang what the search-paths are for include files, normalizes
// them, and returns them in a vector.
static vector<HeaderSearchPath> ComputeHeaderSearchPaths(
    HeaderSearch* header_search) {
  map<string, HeaderSearchPath::Type> search_path_map;
  for (auto it = header_search->system_dir_begin();
       it != header_search->system_dir_end(); ++it) {
    if (OptionalDirectoryEntryRef entry = it->getDirRef()) {
      const string path = NormalizeDirPath(MakeAbsolutePath(entry->getName().str()));
      search_path_map[path] = HeaderSearchPath::kSystemPath;
    }
  }
  for (auto it = header_search->search_dir_begin();
       it != header_search->search_dir_end(); ++it) {
    if (OptionalDirectoryEntryRef entry = it->getDirRef()) {
      // search_dir_begin()/end() includes both system and user paths.
      // If it's a system path, it's already in the map, so everything
      // new is a user path.  The insert only 'takes' for new entries.
      const string path = NormalizeDirPath(MakeAbsolutePath(entry->getName().str()));
      search_path_map.insert(make_pair(path, HeaderSearchPath::kUserPath));
    }
  }
  return NormalizeHeaderSearchPaths(search_path_map);
}

static CStdLib DeriveCStdLib() {
  if (GlobalFlags().no_internal_mappings)
    return CStdLib::None;
  if (GlobalFlags().HasExperimentalFlag("clang_mappings"))
    return CStdLib::ClangSymbols;
  return CStdLib::Glibc;
}

static CXXStdLib DeriveCXXStdLib(const CompilerInstance& compiler,
                                 const ToolChain& toolchain) {
  if (GlobalFlags().no_internal_mappings || !compiler.getLangOpts().CPlusPlus)
    return CXXStdLib::None;
  if (GlobalFlags().HasExperimentalFlag("clang_mappings"))
    return CXXStdLib::ClangSymbols;

  // Get standard library requested for the compilation. ToolChain caches the
  // already-parsed args, so pass in an empty arglist.
  llvm::opt::InputArgList nullargs;
  switch (toolchain.GetCXXStdlibType(nullargs)) {
    case ToolChain::CXXStdlibType::CST_Libcxx:
      return CXXStdLib::Libcxx;
    case ToolChain::CXXStdlibType::CST_Libstdcxx:
      return CXXStdLib::Libstdcxx;
  }
  CHECK_UNREACHABLE_("covered switch for CXXStdlibType above");
}

void InitGlobals(CompilerInstance& compiler, const ToolChain& toolchain) {
  source_manager = &compiler.getSourceManager();
  data_getter = new SourceManagerCharacterDataGetter(*source_manager);
  vector<HeaderSearchPath> search_paths = ComputeHeaderSearchPaths(
      &compiler.getPreprocessor().getHeaderSearchInfo());
  SetHeaderSearchPaths(search_paths);

  RegexDialect regex_dialect = GlobalFlags().regex_dialect;
  CStdLib cstdlib = DeriveCStdLib();
  CXXStdLib cxxstdlib = DeriveCXXStdLib(compiler, toolchain);
  include_picker = new IncludePicker(regex_dialect, cstdlib, cxxstdlib);

  function_calls_full_use_cache = new FullUseCache;
  class_members_full_use_cache = new FullUseCache;

  for (const HeaderSearchPath& entry : search_paths) {
    const char* path_type_name =
        (entry.path_type == HeaderSearchPath::kSystemPath ? "system" : "user");
    VERRS(6) << "Search path: " << entry.path << " (" << path_type_name
             << ")\n";
  }

  // Add mappings.
  for (const string& mapping_file : GlobalFlags().mapping_files) {
    include_picker->AddMappingsFromFile(mapping_file);
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

SourceManager* GlobalSourceManager() {
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

const PrintingPolicy& DefaultPrintPolicy() {
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
  commandline_flags->check_also.insert(NormalizeFilePath(glob));
}

bool ShouldReportIWYUViolationsFor(OptionalFileEntryRef file) {
  const string filepath = GetFilePath(file);
  for (const string& glob : GlobalFlags().check_also)
    if (GlobMatchesPath(glob.c_str(), filepath.c_str()))
      return true;
  return false;
}

void AddGlobToKeepIncludes(const string& glob) {
  CHECK_(commandline_flags && "Call ParseIwyuCommandlineFlags() before this");
  commandline_flags->keep.insert(NormalizeFilePath(glob));
}

bool ShouldKeepIncludeFor(OptionalFileEntryRef file) {
  if (GlobalFlags().keep.empty())
    return false;
  const string filepath = GetFilePath(file);
  for (const string& glob : GlobalFlags().keep)
    if (GlobMatchesPath(glob.c_str(), filepath.c_str()))
      return true;
  return false;
}

void InitGlobalsAndFlagsForTesting() {
  CHECK_(commandline_flags == nullptr && "Only parse commandline flags once");
  CHECK_(include_picker == nullptr && "Only call InitGlobals[ForTesting] once");
  commandline_flags = new CommandlineFlags;
  source_manager = nullptr;
  data_getter = nullptr;
  CStdLib cstdlib = CStdLib::Glibc;
  CXXStdLib cxxstdlib = CXXStdLib::Libstdcxx;
  if (GlobalFlags().no_internal_mappings) {
    cstdlib = CStdLib::None;
    cxxstdlib = CXXStdLib::None;
  } else if (GlobalFlags().HasExperimentalFlag("clang_mappings")) {
    cstdlib = CStdLib::ClangSymbols;
    cxxstdlib = CXXStdLib::ClangSymbols;
  }

  include_picker =
      new IncludePicker(GlobalFlags().regex_dialect, cstdlib, cxxstdlib);

  function_calls_full_use_cache = new FullUseCache;
  class_members_full_use_cache = new FullUseCache;

  // Use a reasonable default for the -I flags.
  map<string, HeaderSearchPath::Type> search_path_map;
  search_path_map["/usr/include/"] = HeaderSearchPath::kSystemPath;
  search_path_map["/usr/include/c++/4.3/"] = HeaderSearchPath::kSystemPath;
  search_path_map["/usr/include/c++/4.2/"] = HeaderSearchPath::kSystemPath;
  search_path_map["./"] = HeaderSearchPath::kUserPath;
  search_path_map["/usr/src/linux-headers-2.6.24-gg23/include/"] =
      HeaderSearchPath::kSystemPath;

  SetHeaderSearchPaths(NormalizeHeaderSearchPaths(search_path_map));
}

}  // namespace include_what_you_use
