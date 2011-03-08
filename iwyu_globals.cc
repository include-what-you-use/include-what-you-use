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
#include "iwyu_include_picker.h"
#include "iwyu_lexer_utils.h"
#include "iwyu_location_util.h"
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
static set<string>* globs_to_report_iwyu_violations_for = NULL;

static vector<string>* MakeSystemDirs(clang::HeaderSearch* header_search) {
  vector<string>* system_include_dirs = new vector<string>;
  for (clang::HeaderSearch::search_dir_iterator
           it = header_search->search_dir_begin();
       it != header_search->search_dir_end(); ++it) {
    const DirectoryEntry * entry = it->getDir();
    if (entry != NULL) {
      system_include_dirs->push_back(entry->getName());
    }
  }
  return system_include_dirs;
}

void InitGlobals(clang::SourceManager* sm, clang::HeaderSearch* header_search) {
  source_manager = sm;
  data_getter = new SourceManagerCharacterDataGetter(*source_manager);
  search_paths = MakeSystemDirs(header_search);
  include_picker = new IncludePicker;
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

}  // namespace include_what_you_use
