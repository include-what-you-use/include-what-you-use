//===--- iwyu_path_util.h - file-path utilities for include-what-you-use --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// File-path utilities for the IWYU checker.

#ifndef INCLUDE_WHAT_YOU_USE_IWYU_PATH_UTIL_H_
#define INCLUDE_WHAT_YOU_USE_IWYU_PATH_UTIL_H_

#include <string>                       // for string, allocator, etc
#include <vector>

#include "llvm/ADT/StringRef.h"

namespace include_what_you_use {

using llvm::StringRef;
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

// Returns true if 'path' is a path of a C++ header file.
bool IsHeaderFilename(StringRef path);

// Returns true if 'quoted_include' contains a path of a C++ header file.
bool IsQuotedHeaderFilename(StringRef quoted_include);

// If the path has a slash, return the part after the last slash,
// else return the input path.
string Basename(StringRef path);

// Normalizes the file path, then strips uninteresting suffixes from
// the file name. Replaces "/internal/" with "/public/" and
// "/include/" with "/src".
string GetCanonicalName(string file_path);

// Replaces "\" by "/" (Microsoft platform paths) and collapses all dot
// components in path.
string NormalizeFilePath(StringRef path);

// Normalizes like NormalizeFilePath and ensures trailing slash.
// Hence use only for directories!
string NormalizeDirPath(StringRef path);

// Is path absolute?
bool IsAbsolutePath(StringRef path);

// Get absolute version of path.
string MakeAbsolutePath(StringRef path);
string MakeAbsolutePath(StringRef base_path, StringRef relative_path);

// Get the parent of path.
string GetParentPath(StringRef path);

// Try to strip the prefix_path from the front of path.
// The path assumed to be normalized but either absolute or relative.
// Return true if path was stripped.
bool StripPathPrefix(string* path, StringRef prefix_path);

// Below, we talk 'quoted' includes.  A quoted include is something
// that would be written on an #include line, complete with the <> or
// "".  In the line '#include <time.h>', "<time.h>" is the quoted
// include.

// Converts a file-path, such as /usr/include/stdio.h, to a
// quoted include, such as <stdio.h>.
string ConvertToQuotedInclude(StringRef filepath,
                              StringRef includer_path = {});

// Returns true if the string is a quoted include.
bool IsQuotedInclude(StringRef s);

// Returns include name enclosed in double quotes or angle quotes, depending on
// the angled flag. An include name is the unquoted relative name that would be
// used on an include line, e.g. lib/mytype.h or stdio.h.
string AddQuotes(string include_name, bool angled);

// Returns true if argument is one of the special filenames used by Clang for
// implicit buffers ("<built-in>", "<command-line>", etc).
bool IsSpecialFilename(StringRef name);

// Append path to dirpath.
string PathJoin(StringRef dirpath, StringRef relative_path);

}  // namespace include_what_you_use

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_PATH_UTIL_H_
