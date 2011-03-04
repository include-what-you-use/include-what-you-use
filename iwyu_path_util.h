//===--- iwyu_path_util.h - file-path utilities for include-what-you-use --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// File-path utilities for the IWYU checker.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_PATH_UTIL_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_PATH_UTIL_H_

#include <limits.h>  // for PATH_MAX
#include <string.h>
#if defined(_MSC_VER)
#include <direct.h>
#else
#include <unistd.h>  // for getcwd()
#endif
#include <string>
#include "port.h"
#include "iwyu_string_util.h"

namespace include_what_you_use {

using std::string;


// Returns true if 'path' is a path of a (possibly enclosed in double
// quotes or <>) C++ header file.
inline bool IsHeaderFile(string path) {
  if (EndsWith(path, "\"") || EndsWith(path, ">"))
    path = path.substr(0, path.length() - 1);

  // Some headers don't have an extension (e.g. <string>), or have an
  // unusual one (the compiler doesn't care), so it's safer to
  // enumerate non-header extensions instead.
  if (EndsWith(path, ".cc") || EndsWith(path, ".c") ||
      EndsWith(path, ".cxx") || EndsWith(path, ".cpp"))
    return false;
  return true;
}

inline string GetCWD() {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)))
    return cwd;
  return "";
}

inline string NormalizeFilePath(const string& path) {
  string result = path;
  while (StripLeft(&result, "./")) {
  }
  return result;
}

inline string CanonicalizeFilePath(const string& path) {
  string result = path;

#ifdef _MSC_VER
  // canonicalise directory separators (forward slashes considered canonical)
  for (size_t i = 0; i < result.size(); ++i) {
    if (result[i] == '\\')
      result[i] = '/';
  }
#endif

  // We may also want to collapse ../ here.

  return result;
}

// Strips uninteresting suffixes from the file name.
inline string GetCanonicalName(string file_path) {
  // Get rid of any <> and "" in case file_path is really an #include line.
  StripLeft(&file_path, "\"") || StripLeft(&file_path, "<");
  StripRight(&file_path, "\"") || StripRight(&file_path, ">");

  file_path = CanonicalizeFilePath(file_path);

  StripRight(&file_path, ".h")
      || StripRight(&file_path, ".cxx")
      || StripRight(&file_path, ".cpp")
      || StripRight(&file_path, ".cc")
      || StripRight(&file_path, ".c");
  StripRight(&file_path, "_unittest")
      || StripRight(&file_path, "_regtest")
      || StripRight(&file_path, "_test");
  StripRight(&file_path, "-inl");
  // .h files in /public/ match .cc files in /internal/
  const string::size_type internal_pos = file_path.find("/internal/");
  if (internal_pos != string::npos)
    file_path = (file_path.substr(0, internal_pos) + "/public/" +
                 file_path.substr(internal_pos + strlen("/internal/")));
  return file_path;
}

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_PATH_UTIL_H_
