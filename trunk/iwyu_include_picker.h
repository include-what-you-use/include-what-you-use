//===--- iwyu_include_picker.h - map to canonical #includes for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The include-picker provides a list of candidate #include-lines
// that iwyu can suggest in order to include a particular symbol
// or file.
//
// It seems like the 'file' case would be easy ("to include
// /usr/include/math.h, say '#include <math.h>"), but it's
// not because many header files are private, and should not
// be included by users directly.  A private header will have
// one or (occassionally) more public headers that it maps to.
// The include-picker keeps track of these mappings.
//
// It's also possible for a public file to have an include-picker
// mapping.  This means: "it's ok to #include this file directly, but
// you can also get the contents of this file by #including this other
// file as well."  One example is that <ostream> maps to both
// <ostream> and <iostream>.  Other parts of iwyu can decide which
// #include to suggest based on its own heuristics (whether the file
// already needs to #include <iostream> for some other reason, for
// instance).
//
// Some of these mappings are hard-coded, based on my own examination
// of gcc headers on ubuntu.  Some mappings are determined at runtime,
// based on #pragmas or other writeup in the source files themselves.
//
// Mapping a symbol to a file has the same issues.  In most cases, a
// symbol maps to the file that defines it, and iwyu_include_picker
// has nothing useful to say.  But some symbols -- which we hard-code
// -- can be provided by several files.  NULL is a canonical example
// of this.
//
// The include-picker also provides some helper functions for
// converting from file-paths to #include paths, including, routines to
// normalize a file-path to get rid of /usr/include/ prefixes.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_INCLUDE_PICKER_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_INCLUDE_PICKER_H_

#include <map>
#include <set>
#include <string>
#include <vector>
#include "port.h"

namespace include_what_you_use {

using std::map;
using std::set;
using std::string;
using std::vector;


// Below, we talk 'quoted' includes.  A quoted include is something
// that would be written on an #include line, complete with the <> or
// "".  In the line '#include <time.h>', "<time.h>" is the quoted
// include.

// Converts a file-path, such as /usr/include/stdio.h, to a
// quoted include, such as <stdio.h>.
string ConvertToQuotedInclude(const string& filepath);

// Returns whether this is a system (as opposed to user) include
// file, based on where it lives.
bool IsSystemIncludeFile(const string& filepath);

class IncludePicker {
 public:
  enum Visibility { kUnusedVisibility, kPublic, kPrivate };

  // If we map from A to B, it means that every time we need a
  // symbol from A, we can also get it from B.  Another way
  // to think about it is that map_to "re-exports" all the
  // symbols from map_from.
  struct IncludeMapEntry {      // A POD so we can make the input static
    const char* map_from;       // A quoted-include
    Visibility from_visibility;
    const char* map_to;         // A quoted-include
    Visibility to_visibility;
  };

  // For every from-include, we need to know all its associated
  // to-includes (that is, everyone that 're-exports' the
  // from-include).  For simplicity, we just put map-to in every
  // value, which wastes space but doesn't require another struct.
  // I don't think space is an issue here.
  struct IncludeMapValue {
    string map_to;
    Visibility from_visibility;
    Visibility to_visibility;
  };
  typedef map<string, vector<IncludeMapValue> > IncludeMap;  // key is map_from

  IncludePicker();

  // ----- Routines to dynamically modify the include-picker

  // Call this for every #include seen during iwyu analysis.  The
  // include-picker can use this data to better suggest #includes,
  // perhaps.  include_name_as_typed includes the <> or "".
  void AddDirectInclude(const string& includer_filepath,
                        const string& include_name_as_typed);

  // Add this to say "map_to re-exports everything in file map_from".
  // Both map_to and map_from should be quoted includes.
  void AddMapping(const string& map_from, Visibility from_visibility,
                  const string& map_to, Visibility to_visibility);

  // Call this after iwyu preprocessing is done.  No more calls to
  // AddDirectInclude() or AddMapping() are allowed after this.
  void FinalizeAddedIncludes();

  // ----- Include-picking API

  // Returns the set of all public header files that 'provide' the
  // given symbol.  For instance, NULL can map to stddef.h, stdlib.h,
  // etc.  Most symbols don't have pre-defined headers they map to,
  // and we return the empty vector in that case.  Ordering is
  // important (which is why we return a vector, not a set): all else
  // being equal, the first element of the vector is the "best" (or
  // most standard) header for the symbol.
  vector<string> GetPublicHeadersForSymbol(const string& symbol) const;

  // Returns the set of all public header files that a given header
  // file -- specified as a full path -- would map to, as a set of
  // quoted includes such as '<stdio.h>'.  If the include-picker has
  // no mapping information for this file, the return vector has just
  // the input file (now include-quoted).  Ordering is important
  // (which is why we return a vector, not a set): all else being
  // equal, the first element of the vector is the "best" (or most
  // standard) header for the input header.
  vector<string> GetPublicHeadersForFilepath(const string& filepath) const;

  // Returns true if there is a mapping (possibly indirect) from
  // map_from to map_to.  This means that to_file 're-exports' all the
  // symbols from from_file.  Both map_from_filepath and
  // map_to_filepath should be full file-paths.
  bool HasMapping(const string& map_from_filepath,
                  const string& map_to_filepath) const;

 private:
  // Given a map whose keys may have globs (* or [] or ?), expand the
  // globs by matching them against all #includes seen by iwyu.
  void ExpandGlobs();

  // One map from symbols to includes, one from filepaths to includes.
  IncludeMap symbol_include_map_;
  IncludeMap filepath_include_map_;

  // All the includes we've seen so far, to help with globbing.
  set<string> all_quoted_includes_;

  // Make sure we don't do any non-const operations after finalizing.
  bool has_called_finalize_added_include_lines_;
};  // class IncludePicker

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_INCLUDE_PICKER_H_
