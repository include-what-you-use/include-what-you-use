//===--- iwyu_string_util.cpp - global variables for include-what-you-use -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "port.h"
#include "iwyu_globals.h"
#ifndef _MSC_VER      // _MSC_VER gets its own fnmatch from ./port.h
#include <fnmatch.h>
#endif
#include <set>
#include <string>
#include "clang/Lex/HeaderSearch.h"
#include "iwyu_cache.h"
#include "iwyu_include_picker.h"
#include "iwyu_lexer_utils.h"
#include "iwyu_location_util.h"
#include "iwyu_output.h"
#include "iwyu_stl_util.h"

using clang::DirectoryEntry;
using clang::DirectoryLookup;
using std::set;
using std::string;
using std::vector;

namespace include_what_you_use {

static clang::SourceManager* source_manager = NULL;
static vector<string>* search_paths = NULL;
static IncludePicker* include_picker = NULL;
static const clang::LangOptions default_lang_options;
static const clang::PrintingPolicy default_print_policy(default_lang_options);
static SourceManagerCharacterDataGetter* data_getter = NULL;
static FullUseCache* function_calls_full_use_cache = NULL;
static FullUseCache* class_members_full_use_cache = NULL;
static set<string>* globs_to_report_iwyu_violations_for = NULL;

// Make sure we put longer search-paths first, so iwyu will map
// /usr/include/c++/4.4/foo to <foo> rather than <c++/4.4/foo>.
static bool SortByDescendingLength(const string& left, const string& right) {
  return left.length() > right.length();
}

static vector<string>* ComputeSystemIncludeDirectories(
    clang::HeaderSearch* header_search) {
  vector<string>* system_include_dirs = new vector<string>;
  for (clang::HeaderSearch::search_dir_iterator
           it = header_search->search_dir_begin();
       it != header_search->search_dir_end(); ++it) {
    const DirectoryEntry * entry = it->getDir();
    if (entry != NULL) {
      system_include_dirs->push_back(entry->getName());
    }
  }
  sort(system_include_dirs->begin(), system_include_dirs->end(),
       &SortByDescendingLength);
  return system_include_dirs;
}

void InitGlobals(clang::SourceManager* sm, clang::HeaderSearch* header_search) {
  CHECK_(sm && "InitGlobals() needs a non-NULL SourceManager");
  source_manager = sm;
  data_getter = new SourceManagerCharacterDataGetter(*source_manager);
  search_paths = ComputeSystemIncludeDirectories(header_search);
  include_picker = new IncludePicker;
  function_calls_full_use_cache = new FullUseCache;
  class_members_full_use_cache = new FullUseCache;

  for (Each<string> it(search_paths); !it.AtEnd(); ++it)
    VERRS(6) << "Search path: " << *it << "\n";
}

clang::SourceManager* GlobalSourceManager() {
  CHECK_(source_manager && "Must call InitGlobals() before calling this");
  return source_manager;
}

const vector<string>& GlobalSearchPaths() {
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
  if (globs_to_report_iwyu_violations_for == NULL)
    globs_to_report_iwyu_violations_for = new set<string>;
  globs_to_report_iwyu_violations_for->insert(glob);
}

bool ShouldReportIWYUViolationsFor(const clang::FileEntry* file) {
  if (globs_to_report_iwyu_violations_for == NULL)
    return false;
  const string filepath = GetFilePath(file);
  for (Each<string> it(globs_to_report_iwyu_violations_for); !it.AtEnd(); ++it)
    if (fnmatch(it->c_str(), filepath.c_str(), FNM_PATHNAME) == 0)
      return true;
  return false;
}

void InitGlobalsForTesting() {
  source_manager = NULL;
  data_getter = NULL;
  include_picker = new IncludePicker;
  function_calls_full_use_cache = new FullUseCache;
  class_members_full_use_cache = new FullUseCache;

  // Use a reasonable default for the -I flags.
  search_paths = new vector<string>;
  search_paths->push_back("/usr/include");
  search_paths->push_back("/usr/include/c++/4.3");
  search_paths->push_back("/usr/include/c++/4.2");
  search_paths->push_back(".");
  search_paths->push_back("/usr/src/linux-headers-2.6.24-gg23/include");

  sort(search_paths->begin(), search_paths->end(), &SortByDescendingLength);
}

}  // namespace include_what_you_use
