//===--- iwyu_path_util.cc - file-path utilities for include-what-you-use -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_path_util.h"

#include <stddef.h>
#include <string.h>                     // for strlen
#include <system_error>
#include <limits.h>
#include <cstdlib>

#include "iwyu_stl_util.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace include_what_you_use {

namespace {
vector<HeaderSearchPath>* header_search_paths;

// Please keep this in sync with _SOURCE_EXTENSIONS in fix_includes.py.
const char* source_extensions[] = {
  ".c",
  ".C",
  ".cc",
  ".CC",
  ".cxx",
  ".CXX",
  ".cpp",
  ".CPP"
  ".c++",
  ".C++",
  ".cp"
};

}  // namespace

void SetHeaderSearchPaths(const vector<HeaderSearchPath>& search_paths) {
  if (header_search_paths != NULL) {
    delete header_search_paths;
  }
  header_search_paths = new vector<HeaderSearchPath>(search_paths);
}

const vector<HeaderSearchPath>& HeaderSearchPaths() {
  if (header_search_paths == NULL) {
    header_search_paths = new vector<HeaderSearchPath>();
  }
  return *header_search_paths;
}

bool IsHeaderFile(string path) {
  if (EndsWith(path, "\"") || EndsWith(path, ">"))
    path = path.substr(0, path.length() - 1);

  // Some headers don't have an extension (e.g. <string>), or have an
  // unusual one (the compiler doesn't care), so it's safer to
  // enumerate non-header extensions instead.
  for (size_t i = 0; i < llvm::array_lengthof(source_extensions); ++i) {
    if (EndsWith(path, source_extensions[i]))
      return false;
  }

  return true;
}

string Basename(const string& path) {
  string::size_type last_slash = path.rfind('/');
  if (last_slash != string::npos) {
    return path.substr(last_slash + 1);
  }
  return path;
}

string CanonicalizeFilePath(const string& path) {
  string result = path;

#ifdef _WIN32
  // Canonicalise directory separators (forward slashes considered canonical.)
  for (size_t i = 0; i < result.size(); ++i) {
    if (result[i] == '\\')
      result[i] = '/';
  }
  // We may also want to collapse ../ here.
#else
  char realpath_result[PATH_MAX + 1];
  if (NULL != realpath(path.c_str(), realpath_result)) {
      result = string(realpath_result);
  }
#endif

  return result;
}

string CanonicalizeHeaderSearchPath(const string& path) {
  string result = CanonicalizeFilePath(path);

  // We want a trailing slash on all header search paths, because it makes it
  // much easier to find the longest common path prefix.
  if (!EndsWith(result, "/"))
    result += "/";

  return result;
}

string GetCanonicalName(string file_path) {
  // Get rid of any <> and "" in case file_path is really an #include line.
  StripLeft(&file_path, "\"") || StripLeft(&file_path, "<");
  StripRight(&file_path, "\"") || StripRight(&file_path, ">");

  file_path = CanonicalizeFilePath(file_path);

  bool stripped_ext = StripRight(&file_path, ".h")
      || StripRight(&file_path, ".hpp")
      || StripRight(&file_path, ".hxx")
      || StripRight(&file_path, ".hh")
      || StripRight(&file_path, ".inl");
  if (!stripped_ext) {
    for (size_t i = 0; i < llvm::array_lengthof(source_extensions); ++i) {
      if (StripRight(&file_path, source_extensions[i]))
        break;
    }
  }

  StripRight(&file_path, "_unittest")
      || StripRight(&file_path, "_regtest")
      || StripRight(&file_path, "_test")
      || StripLeft(&file_path, "test_headercompile_");
  StripRight(&file_path, "-inl");
  // .h files in /public/ match .cc files in /internal/
  const string::size_type internal_pos = file_path.find("/internal/");
  if (internal_pos != string::npos)
    file_path = (file_path.substr(0, internal_pos) + "/public/" +
                 file_path.substr(internal_pos + strlen("/internal/")));

  // .h files in /include/ match .cc files in /src/
  const string::size_type include_pos = file_path.find("/include/");
  if (include_pos != string::npos)
    file_path = (file_path.substr(0, include_pos) + "/src/" +
                 file_path.substr(include_pos + strlen("/include/")));
  return file_path;
}

string NormalizeFilePath(const string& path) {
  string result = CanonicalizeFilePath(path);
  while (StripLeft(&result, "./")) {
  }
  return result;
}

bool IsAbsolutePath(const string& path) {
  return llvm::sys::path::is_absolute(path);
}

string MakeAbsolutePath(const string& path) {
  llvm::SmallString<128> absolute_path(path.c_str());
  std::error_code error = llvm::sys::fs::make_absolute(absolute_path);
  CHECK_(!error);

  return absolute_path.str();
}

string MakeAbsolutePath(const string& base_path, const string& relative_path) {
  llvm::SmallString<128> absolute_path(base_path.c_str());
  llvm::sys::path::append(absolute_path, relative_path);

  return absolute_path.str();
}

string GetParentPath(const string& path) {
  llvm::StringRef parent = llvm::sys::path::parent_path(path);
  return parent.str();
}

// Converts a file-path, such as /usr/include/stdio.h, to a
// quoted include, such as <stdio.h>.
string ConvertToQuotedInclude(const string& filepath) {
  // First, get rid of leading ./'s and the like.
  string path = NormalizeFilePath(filepath);

  // Case 1: Uses an explicit entry on the search path (-I) list.
  const vector<HeaderSearchPath>& search_paths = HeaderSearchPaths();
  // HeaderSearchPaths is sorted to be longest-first, so this
  // loop will prefer the longest prefix: /usr/include/c++/4.4/foo
  // will be mapped to <foo>, not <c++/4.4/foo>.
  for (Each<HeaderSearchPath> it(&search_paths); !it.AtEnd(); ++it) {
    // All header search paths have a trailing "/", so we'll get a perfect
    // quoted include by just stripping the prefix.
    if (StripLeft(&path, it->path)) {
      if (it->path_type == HeaderSearchPath::kSystemPath)
        return "<" + path + ">";
      else
        return "\"" + path + "\"";
    }
  }

  // Case 2: Uses the implicit "-I." entry on the search path.  Always local.
  return "\"" + path + "\"";
}

bool IsQuotedInclude(const string& s) {
  if (s.size() < 2)
    return false;
  return ((StartsWith(s, "<") && EndsWith(s, ">")) ||
          (StartsWith(s, "\"") && EndsWith(s, "\"")));
}

// Returns whether this is a system (as opposed to user) include file,
// based on where it lives.
bool IsSystemIncludeFile(const string& filepath) {
  return ConvertToQuotedInclude(filepath)[0] == '<';
}

// Returns true if the given file is third-party.  Google-authored
// code living in third_party/ is not considered third-party.
bool IsThirdPartyFile(string quoted_path) {
  if (!StripLeft(&quoted_path, "\"third_party/"))
    return false;

  // These are Google-authored libraries living in third_party/
  // because of old licensing constraints.
  if (StartsWith(quoted_path, "car/") ||
      StartsWith(quoted_path, "gtest/") ||
      StartsWith(quoted_path, "gmock/"))
    return false;

  return true;
}

}  // namespace include_what_you_use
