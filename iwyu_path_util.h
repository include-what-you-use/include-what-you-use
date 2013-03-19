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

#include <string>                       // for string, allocator, etc
#include <vector>

#include "iwyu_string_util.h"

namespace include_what_you_use {

using std::string;
using std::vector;


// One entry in the search-path list of where to find #include files.
struct HeaderSearchPath {
  enum Type { kUnusedPath = 0, kSystemPath, kUserPath };
  HeaderSearchPath(const string& p, Type pt) : path(p), path_type(pt) { }
  string path;      // the path-entry as specified on the commandline (via -I)
  Type path_type;
};

// The directories to look for #includes in, including from -I, -isystem, etc.
void SetHeaderSearchPaths(const vector<HeaderSearchPath>& search_paths);
const vector<HeaderSearchPath>& HeaderSearchPaths();

// Returns true if 'path' is a path of a (possibly enclosed in double
// quotes or <>) C++ header file.
bool IsHeaderFile(string path);

// If the path has a slash, return the part after the last slash,
// else return the input path.
string Basename(const string& path);

// On Microsoft platforms, convert \ to /.
string CanonicalizeFilePath(const string& path);

// Removes enclosing <> or "", then strips uninteresting suffixes from
// the file name. Replaces "/internal/" with "/public/" and
// "/include/" with "/src".  "Canonicalize" the path on Microsoft
// platforms.
string GetCanonicalName(string file_path);

// "Canonicals" the name on Microsoft platforms, then recursively
// removes all "./" prefixes.
string NormalizeFilePath(const string& path);

// Is path absolute?
bool IsAbsolutePath(const string& path);

// Get absolute version of path.
string MakeAbsolutePath(const string& path);
string MakeAbsolutePath(const string& base_path, const string& relative_path);

// Get the parent of path.
string GetParentPath(const string& path);

// Below, we talk 'quoted' includes.  A quoted include is something
// that would be written on an #include line, complete with the <> or
// "".  In the line '#include <time.h>', "<time.h>" is the quoted
// include.

// Converts a file-path, such as /usr/include/stdio.h, to a
// quoted include, such as <stdio.h>.
string ConvertToQuotedInclude(const string& filepath);

// Returns true if the string is a quoted include.
bool IsQuotedInclude(const string& s);

// Returns whether this is a system (as opposed to user) include
// file, based on where it lives.
bool IsSystemIncludeFile(const string& filepath);

// Returns true if the given file is third-party.  Google-authored
// code living in third_party/ is not considered third-party.
bool IsThirdPartyFile(string quoted_path);

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_PATH_UTIL_H_
