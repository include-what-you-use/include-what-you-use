//===--- iwyu_include_picker.h - map to canonical #includes for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Given a symbol, and a file-path where the symbol is defined (or
// possibly just declared), return the best quoted include path to
// get that symbol.  A "quoted include path" is something that can
// be used with an #include, including surrounding ""'s or <>'s.
//
// Picking the best #include has several steps:
// 1) Normalizing the path to get rid of /usr/include/ prefixes and the
//    like, and adding ""s or <>'s.
// 2) Mapping internal-only header files to the public header file that
//    should be used instead.  For instance, this will map
//    /usr/include/.../bits/stl_vector.h to <vector>.
// 3) For internal-only header files that could come from a number
//    of public header files, picking the best header file to return
//    (based on lists of "preferred" includes the client provides).
// 4) For symbols that could come from a number of public header files
//    -- for instance, sigset_t can come either from sys/select.h or
//    sys/epoll.h -- pick the best header file to return (again
//    based on lists of "preferred" includes the client provides).
//
// We also make public some of the helper routines, for instance
// SanitizePath() which just does steps 1-3 but doesn't require
// a symbol name, and IsSystemIncludeFile(), which does enough
// of step 1 to determine if we'll be adding ""'s or <>'s.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_INCLUDE_PICKER_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_INCLUDE_PICKER_H_

#include <map>
#include <set>
#include <string>
#include <vector>
#include "port.h"

namespace include_what_you_use {

using std::map;
using std::multimap;
using std::set;
using std::string;
using std::vector;


class IncludePicker {
 public:
  // The key is 'name' below, and the value is 'path'.
  typedef multimap<string, string> IncludeMap;
  struct IncludeMapEntry {   // A POD so we can make the input static
    const char* name;        // An unquoted but normalized input include loc
    const char* path;        // A quoted output include loc, e.g. "<string>"
  };

  IncludePicker();

  // Call this for every #include seen during iwyu analysis.  The
  // include-picker can use this data to better suggest #includes,
  // perhaps.  include_name_as_typed includes the <> or "".
  void AddDirectInclude(const string& includer,
                        const string& include_name_as_typed);

  // Used to register mappings from private header files to public
  // header files at runtime.  This is intended to be used when
  // files advertise these mappings via a #pragma or some such.
  void AddPrivateToPublicMapping(const string& quoted_private_header,
                                 const string& public_header);

  // Call this after iwyu preprocessing is done.  No more calls to
  // AddDirectInclude() or AddPrivateToPublicMapping() are allowed
  // after this.
  void FinalizeAddedIncludes();

  // Removes uninteresting prefix from the given header path to make
  // it like what a user would write on a #include line; returns the
  // unquoted, stripped path.  This is a helper function for
  // GetBestIncludeForSymbol, and probably will rarely need to be
  // called directly.
  // Note: 'string' rather than 'const string&' since this fn munges the arg.
  string StripPathPrefix(string path) const {
    const IncludeMap* dummy;
    StripPathPrefixAndGetAssociatedIncludeMap(&path, &dummy);
    return path;
  }

  // Returns whether this is a system (as opposed to user) include
  // file, based on where it lives.
  // Note: 'string' rather than 'const string&' since this fn munges the arg.
  bool IsSystemIncludeFile(string path) const {
    const IncludeMap* include_map;
    StripPathPrefixAndGetAssociatedIncludeMap(&path, &include_map);
    return IsMapForSystemHeader(include_map);
  }

  // Combines the above two to convert a path into an #include argument
  // that returns that path.  For instance, turns '/usr/include/stdio.h'
  // into '<stdio.h>'.
  // Note: 'string' rather than 'const string&' since this fn munges the arg.
  string GetQuotedIncludeFor(string path) const {
    const IncludeMap* include_map;
    StripPathPrefixAndGetAssociatedIncludeMap(&path, &include_map);
    return IsMapForSystemHeader(include_map) ? "<"+path+">" : "\""+path+"\"";
  }

  // Returns the set of all public header files that a given symbol
  // would map to.  For instance, NULL can map to stddef.h, stdlib.h,
  // etc.  Most symbols don't have pre-defined headers they map to,
  // and we return the empty vector in that case.  Ordering is
  // important (which is why we return a vector, not a set): all else
  // being equal, the first element of the vector is the "best" (or
  // most standard) header for the symbol.
  vector<string> GetPublicHeadersForSymbol(const string& symbol) const;

  // Returns the set of all public header files that a given header
  // file -- specified as a full path -- would map to.  This set
  // contains just the input header file (now quoted) if the given
  // header file is not a private header file.  The entries of the set
  // are quoted includes, such as '<stdio.h>'.  Ordering is important
  // (which is why we return a vector, not a set): all else being
  // equal, the first element of the vector is the "best" (or most
  // standard) header for the private header.
  // Note: 'string' rather than 'const string&' since this fn munges the arg.
  vector<string> GetPublicHeadersForPrivateHeader(string private_header) const;

  // Returns true iff there is evidence that the includer means to
  // re-export the includee.  Right now, the only evidence we have is
  // if there's an include-picker mapping from includee_path to
  // includer_path (each converted from a path to an include-name
  // first, of course).  Note that, due to lack of evidence, this
  // function may return false in some cases where the includer means
  // to re-export some or all symbols from the includee.
  // Note: 'string' rather than 'const string&' since this fn munges the arg.
  bool PathReexportsInclude(string includer_path, string includee_path) const;

 private:
  bool IsMapForSystemHeader(const IncludeMap* include_map) const {
    return include_map == &cpp_include_map_ || include_map == &c_include_map_;
  }

  // Removes uninteresting prefix from the given header path to make
  // it like what a user would write on a #include line, and sets
  // *include_map to the IncludeMap associated with the header.
  void StripPathPrefixAndGetAssociatedIncludeMap(
      string* path, const IncludeMap** include_map) const;

  // Maps for system headers.
  const IncludeMap cpp_include_map_;
  const IncludeMap c_include_map_;

  // Maps for user headers.
  const IncludeMap google_include_map_;
  const IncludeMap third_party_include_map_;

  // A map that is constructed at runtime based on the #includes
  // actually seen while processing a source file.  It includes, for
  // instance, mappings from 'project/internal/foo.h' to
  // 'project/public/foo_public.h' in google code (Google hides
  // private headers in /internal/, much like glibc hides them in
  // /bits/.)  This dynamic mapping is not as good as the hard-coded
  // mappings, since it has incomplete information (only what is seen
  // during this compile run) and has to do a best-effort guess at the
  // mapping.  And we still have to hard-code in some rules to tell
  // whether a file is public or private from its name (which we do in
  // AddDirectInclude).  It's a fallback map when other maps come up
  // empty.
  IncludeMap dynamic_private_to_public_include_map_;

  // Maps from qualified symbols to headers.
  const IncludeMap symbol_include_map_;
};  // class IncludePicker

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_INCLUDE_PICKER_H_
