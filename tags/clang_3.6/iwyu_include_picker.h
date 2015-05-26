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

#include <map>                          // for map, map<>::value_compare
#include <set>                          // for set
#include <string>                       // for string
#include <utility>                      // for pair
#include <vector>                       // for vector

namespace clang {
class FileEntry;
}  // namespace clang

namespace include_what_you_use {

using std::map;
using std::pair;
using std::set;
using std::string;

using std::vector;

struct IncludeMapEntry;

enum IncludeVisibility { kUnusedVisibility, kPublic, kPrivate };

class IncludePicker {
 public:
  typedef map<string, vector<string> > IncludeMap;  // map_from to <map_to,...>

  explicit IncludePicker(bool no_default_mappings);

  // ----- Routines to dynamically modify the include-picker

  // Call this for every #include seen during iwyu analysis.  The
  // include-picker can use this data to better suggest #includes,
  // perhaps.
  void AddDirectInclude(const string& includer_filepath,
                        const string& includee_filepath,
                        const string& quoted_include_as_written);

  // Add this to say "map_to re-exports everything in file map_from".
  // Both map_to and map_from should be quoted includes.
  void AddMapping(const string& map_from, const string& map_to);

  // Indicate that the given quoted include should be considered
  // a "private" include.  If possible, we use the include-picker
  // mappings to map such includes to public (not-private) includs.
  void MarkIncludeAsPrivate(const string& quoted_include);

  // Add this to say that "any file whose name matches the
  // friend_regex is allowed to include includee_filepath".  The regex
  // uses the POSIX Entended Regular Expression syntax and should
  // match a quoted-include (starting and ending with "" or <>).
  void AddFriendRegex(const string& includee_filepath,
                      const string& quoted_friend_regex);

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
  vector<string> GetCandidateHeadersForSymbol(const string& symbol) const;

  // Returns the set of all public header files that a given header
  // file -- specified as a full path -- would map to, as a set of
  // quoted includes such as '<stdio.h>'.  If the include-picker has
  // no mapping information for this file, the return vector has just
  // the input file (now include-quoted).  Ordering is important
  // (which is why we return a vector, not a set): all else being
  // equal, the first element of the vector is the "best" (or most
  // standard) header for the input header.
  vector<string> GetCandidateHeadersForFilepath(const string& filepath) const;

  // This allows for special-casing of GetCandidateHeadersForFilepath
  // -- it's the same, but you give it the filepath that's doing the
  // #including.  This lets us give a different answer for different
  // call-sites.  For instance, "foo/internal/bar.h" is a fine
  // candidate header when #included from "foo/internal/baz.h", but
  // not when #included from "qux/quux.h".  In the common case there's
  // no special-casing, and this falls back on
  // GetCandidateHeadersForFilepath().
  vector<string> GetCandidateHeadersForFilepathIncludedFrom(
      const string& included_filepath, const string& including_filepath) const;

  // Returns true if there is a mapping (possibly indirect) from
  // map_from to map_to.  This means that to_file 're-exports' all the
  // symbols from from_file.  Both map_from_filepath and
  // map_to_filepath should be full file-paths.
  bool HasMapping(const string& map_from_filepath,
                  const string& map_to_filepath) const;

  bool IsPublic(const clang::FileEntry* file) const;

  // Parses a YAML/JSON file containing mapping directives of various types.
  void AddMappingsFromFile(const string& filename);

 private:
  // Private implementation of mapping file parser, which takes
  // mapping file search path to allow recursion that builds up
  // search path incrementally.
  void AddMappingsFromFile(const string& filename,
                           const vector<string>& search_path);

  // Adds all hard-coded default mappings.
  void AddDefaultMappings();

  // Adds a mapping from a one header to another, typically
  // from a private to a public quoted include.
  void AddIncludeMapping(
      const string& map_from, IncludeVisibility from_visibility, 
      const string& map_to, IncludeVisibility to_visibility);

  // Adds a mapping from a a symbol to a quoted include. We use this to 
  // maintain mappings of documented types, e.g.
  //  For std::map<>, include <map>.
  void AddSymbolMapping(
      const string& map_from, const string& map_to,
      IncludeVisibility to_visibility);

  // Adds mappings from sized arrays of IncludeMapEntry.
  void AddIncludeMappings(const IncludeMapEntry* entries, size_t count);
  void AddSymbolMappings(const IncludeMapEntry* entries, size_t count);

  void AddPublicIncludes(const char** includes, size_t count);

  // Expands the regex keys in filepath_include_map_ and
  // friend_to_headers_map_ by matching them against all source files
  // seen by iwyu.
  void ExpandRegexes();

  // Figure out mappings to add between third-party files, that we
  // guess based on the structure and use of third-party code.
  void AddImplicitThirdPartyMappings();

  // Adds an entry to filepath_visibility_map_, with error checking.
  void MarkVisibility(const string& quoted_include, IncludeVisibility vis);

  // Parse visibility from a string. Returns kUnusedVisibility if
  // string is not recognized.
  IncludeVisibility ParseVisibility(const string& visibility_str) const;

  // Return the visibility of a given quoted_include if known, else
  // kUnusedVisibility.
  IncludeVisibility GetVisibility(const string& quoted_include) const;

  // For the given key, return the vector of values associated with
  // that key, or an empty vector if the key does not exist in the
  // map, filtering out private files.
  vector<string> GetPublicValues(const IncludeMap& m, const string& key) const;

  // Given an includer-pathname and includee-pathname, return the
  // quoted-include of the includee, as typed in the includer, or
  // "" if it's not found for some reason.
  string MaybeGetIncludeNameAsWritten(const string& includer_filepath,
                                      const string& includee_filepath) const;

  // From symbols to includes.
  IncludeMap symbol_include_map_;

  // From quoted filepath patterns to includes, where a pattern can be
  // either a quoted filepath (e.g. "foo/bar.h" or <a/b.h>) or @
  // followed by a regular expression for matching a quoted filepath
  // (e.g. @"foo/.*").  If key-value pair (pattern, headers) is in
  // this map, it means that any header in 'headers' can be used to
  // get symbols exported by a header matching 'pattern'.
  IncludeMap filepath_include_map_;

  // A map of all quoted-includes to whether they're public or private.
  // Quoted-includes that are not present in this map are assumed public.
  map<string, IncludeVisibility> filepath_visibility_map_;

  // All the includes we've seen so far, to help with globbing and
  // other dynamic mapping.  For each file, we list who #includes it.
  map<string, set<string> > quoted_includes_to_quoted_includers_;

  // Given the filepaths of an includer and includee, give the
  // include-as-written (including <>'s or ""'s) that the includer
  // used to refer to the includee.  We use this to return includes as
  // they were written in the source, when possible.
  map<pair<string, string>, string> includer_and_includee_to_include_as_typed_;

  // Maps from a quoted filepath pattern to the set of files that used
  // a pragma declaring it as a friend.  That is, if foo/bar/x.h has a
  // line "// IWYU pragma: friend foo/bar/.*" then "x.h" will be a
  // member of friend_to_headers_map_["@\"foo/bar/.*\""]. In a
  // postprocessing step, files friend_to_headers_map_ will have
  // regular expressions expanded, e.g. if foo/bar/x.cc is processed,
  // friend_to_headers_map_["foo/bar/x.cc"] will be augmented with the
  // contents of friend_to_headers_map_["@\"foo/bar/.*\""].
  map<string, set<string> > friend_to_headers_map_;

  // Make sure we don't do any non-const operations after finalizing.
  bool has_called_finalize_added_include_lines_;
};  // class IncludePicker

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_INCLUDE_PICKER_H_
