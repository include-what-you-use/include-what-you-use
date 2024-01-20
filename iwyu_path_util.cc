//===--- iwyu_path_util.cc - file-path utilities for include-what-you-use -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_path_util.h"

#include <cstring>                      // for strlen
#include <system_error>

#include "iwyu_port.h"
#include "iwyu_string_util.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
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
  ".CPP",
  ".c++",
  ".C++",
  ".cp"
};

}  // anonymous namespace

void SetHeaderSearchPaths(const vector<HeaderSearchPath>& search_paths) {
  if (header_search_paths != nullptr) {
    delete header_search_paths;
  }
  header_search_paths = new vector<HeaderSearchPath>(search_paths);
}

const vector<HeaderSearchPath>& HeaderSearchPaths() {
  if (header_search_paths == nullptr) {
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
  //  for (size_t i = 0; i < llvm::array_lengthof(source_extensions); ++i) {
  for (const char* source_extension : source_extensions) {
    if (EndsWith(path, source_extension))
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

string GetCanonicalName(string file_path) {
  // Clang special filenames are already canonical.
  // <stdin> is not a special filename, but it's canonical too.
  if (IsSpecialFilename(file_path) || file_path == "<stdin>")
    return file_path;

  // All known special filenames which look like quoted-includes are handled
  // above. Reject anything else that looks like a quoted-include.
  CHECK_(!IsQuotedInclude(file_path));

  file_path = NormalizeFilePath(file_path);

  bool stripped_ext = StripRight(&file_path, ".h")
      || StripRight(&file_path, ".H")
      || StripRight(&file_path, ".hpp")
      || StripRight(&file_path, ".hxx")
      || StripRight(&file_path, ".hh")
      || StripRight(&file_path, ".inl");
  if (!stripped_ext) {
    for (const char* source_extension : source_extensions) {
      if (StripRight(&file_path, source_extension))
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
  llvm::SmallString<128> normalized(path);
  llvm::sys::path::remove_dots(normalized, /*remove_dot_dot=*/true);
  // Canonicalize directory separators (forward slashes considered canonical.)
  return llvm::sys::path::convert_to_slash(normalized);
}

string NormalizeDirPath(const string& path) {
  string result = NormalizeFilePath(path);
  // Ensure trailing slash.
  if (!result.empty() && result.back() != '/')
      result += '/';
  return result;
}

bool IsAbsolutePath(const string& path) {
  return llvm::sys::path::is_absolute(path);
}

string MakeAbsolutePath(const string& path) {
  llvm::SmallString<128> absolute_path(path);
  std::error_code error = llvm::sys::fs::make_absolute(absolute_path);
  CHECK_(!error);

  return absolute_path.str().str();
}

string MakeAbsolutePath(const string& base_path, const string& relative_path) {
  llvm::SmallString<128> absolute_path(base_path);
  llvm::sys::path::append(absolute_path, relative_path);

  return absolute_path.str().str();
}

string GetParentPath(const string& path) {
  llvm::StringRef parent = llvm::sys::path::parent_path(path);
  return parent.str();
}

bool StripPathPrefix(string* path, const string& prefix_path) {
  // Only makes sense if both are absolute or both are relative (to same dir).
  CHECK_(IsAbsolutePath(*path) == IsAbsolutePath(prefix_path));
  return StripLeft(path, prefix_path);
}

// Converts a file-path, such as /usr/include/stdio.h, to a
// quoted include, such as <stdio.h>.
string ConvertToQuotedInclude(const string& filepath,
                              const string& includer_path) {
  // includer_path must be given as an absolute path.
  CHECK_(includer_path.empty() || IsAbsolutePath(includer_path));

  if (filepath == "<built-in>")
    return filepath;

  // Get path into same format as header search paths: Absolute and normalized.
  string path = NormalizeFilePath(MakeAbsolutePath(filepath));

  // Case 1: Uses an explicit entry on the search path (-I) list.
  const vector<HeaderSearchPath>& search_paths = HeaderSearchPaths();
  // HeaderSearchPaths is sorted to be longest-first, so this
  // loop will prefer the longest prefix: /usr/include/c++/4.4/foo
  // will be mapped to <foo>, not <c++/4.4/foo>.
  for (const HeaderSearchPath& entry : search_paths) {
    // All header search paths have a trailing "/", so we'll get a perfect
    // quoted include by just stripping the prefix.

    if (StripPathPrefix(&path, entry.path)) {
      if (entry.path_type == HeaderSearchPath::kSystemPath)
        return "<" + path + ">";
      else
        return "\"" + path + "\"";
    }
  }

  // Case 2:
  // Uses the implicit "-I <basename current file>" entry on the search path.
  if (!includer_path.empty())
    StripPathPrefix(&path, NormalizeDirPath(includer_path));
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

}  // namespace include_what_you_use
