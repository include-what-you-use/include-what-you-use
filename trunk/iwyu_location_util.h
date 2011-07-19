//===--- iwyu_location_util.h - SourceLoc-related utilities for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// clang-File utilities for the IWYU checker.
//
// This file deals with clang's FileEntry and SourceLocation classes,
// and useful utilities on them.  It does not deal with file paths
// per se; for that, see iwyu_path_util.h
//
// In addition to some ad hoc helper routines, in general, for every
// type we consider, we try to provide 3 routines (as appropriate):
//    GetLocation():  convert the object to an appropriate SourceLocation
//    GetFileEntry(): convert the object to the file it's in
//    GetFilePath():  convert the object to the filename it's in
//
//
// Background on Clang data structures:
//
// Clang uses the type FileEntry to identify a physical file in the
// file system.  A FileEntry is created for each source file Clang
// processes.  Clang never creates two FileEntry objects for the same
// file.  Therefore we use const FileEntry* in IWYU as unique IDs for
// files.
//
// Clang's FileID type is a misnomer.  It's actually an ID of a
// particular #include statement.  If a file is #included in two
// places, it will have two different FildIDs.
//
// A SourceLocation is a compact encoding of a location in a
// particular #include of a source file.  From it you can find the
// file path, line number, column number, and which file (if any)
// #includes the file.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_FILE_UTIL_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_FILE_UTIL_H_

#include <string>                       // for string

#include "iwyu_globals.h"
#include "iwyu_path_util.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Token.h"

namespace clang {
class Decl;
class NestedNameSpecifierLoc;
class Stmt;
class TemplateArgumentLoc;
class TypeLoc;
}  // namespace clang

namespace include_what_you_use {

using std::string;


//------------------------------------------------------------
// Helper functions for FileEntry.

// Some symbols are directly defined by the compiler.  For them, the
// definition location points to the "<built-in>" file.
inline bool IsBuiltinFile(const clang::FileEntry* file) {
  return file == NULL;
}

// There are two kinds of symbols that are not defined in the source
// files: the compiler can define some standard symbols
// (e.g. __FILE__), and the user can define macros on the command line
// of the compiler using -D.  A symbol appears to be defined in file
// "<built-in>" in the first case, and "<command line>" in the second.
// IsBuiltinOrCommandLineFile(file) returns true if it's either of the
// two cases.
inline bool IsBuiltinOrCommandLineFile(const clang::FileEntry* file) {
  return IsBuiltinFile(file) || (!strcmp(file->getName(), "<command line>"));
}

inline string GetFilePath(const clang::FileEntry* file) {
  return (IsBuiltinFile(file) ? "<built-in>" :
          NormalizeFilePath(file->getName()));
}

//------------------------------------------------------------
// Helper functions for SourceLocation and FullSourceLoc.

inline clang::FullSourceLoc GetSpellingLoc(clang::SourceLocation loc) {
  clang::FullSourceLoc fullloc(loc, *GlobalSourceManager());
  return fullloc.isValid() ? fullloc.getSpellingLoc() : fullloc;
}

inline clang::FullSourceLoc GetInstantiationLoc(clang::SourceLocation loc) {
  clang::FullSourceLoc fullloc(loc, *GlobalSourceManager());
  return fullloc.isValid() ? fullloc.getInstantiationLoc() : fullloc;
}

inline bool IsInMacro(clang::SourceLocation loc) {
  return GetSpellingLoc(loc) != GetInstantiationLoc(loc);
}

// Returns the line number corresponding to a source location.
// Prefers the spelling line number, but if it's not useful (because
// the location is in <scratch space> for instance, due to macro token
// concatenation), uses the instantiation line number.  Returns -1 if
// we can't figure out the line #.
inline int GetLineNumber(clang::SourceLocation loc) {
  if (!loc.isValid())
    return -1;
  const clang::FullSourceLoc fullloc(loc, *GlobalSourceManager());
  bool invalid = false;
  int retval = fullloc.getSpellingLineNumber(&invalid);
  if (invalid)
    retval = fullloc.getInstantiationLineNumber(&invalid);
  if (invalid)
    retval = -1;
  return retval;
}

// The rest of this section of the file is for returning the
// FileEntry* corresponding to a source location: the file that the
// location is in.  This is a surprising amount of work.

namespace internal {
// clang uses the name FileID to mean 'a filename that was reached via
// a particular series of #includes.'  (What one might think a FileID
// might be -- a unique reference to a filesystem object -- is
// actually a FileEntry*.)  We use a less-confusing pseudonym.
typedef clang::FileID IncludeID;

// Tells which #include loc comes from.
inline IncludeID GetIncludeID(clang::SourceLocation loc) {
  return GlobalSourceManager()->getFileID(loc);
}

inline const clang::FileEntry* GetFileEntryFromIncludeID(IncludeID include_id) {
  return GlobalSourceManager()->getFileEntryForID(include_id);
}
}  // namespace internal

inline const clang::FileEntry* GetFileEntry(clang::FullSourceLoc full_loc) {
  return internal::GetFileEntryFromIncludeID(internal::GetIncludeID(full_loc));
}

inline const clang::FileEntry* GetFileEntry(clang::SourceLocation loc) {
  if (!loc.isValid())
    return NULL;

  // We want where the user actually writes the token, instead of
  // where it appears as part of a macro expansion.  For example, in:
  //
  //  file foo.h,  line 5:  #define FOO(x) x + y
  //  file bar.cc, line 10: FOO(z)
  //
  // FOO(z) will expand to 'z + y', where symbol z's location is
  // foo.h, line 5, and its spelling location is bar.cc, line 10.
  const clang::FileEntry* retval = GetFileEntry(GetSpellingLoc(loc));

  // Sometimes the spelling location is NULL, because the symbol is
  // 'spelled' via macro concatenation.  For instance, all the
  // __gthrw3 symbols in
  // /usr/include/c++/4.2/x86_64-linux-gnu/bits/gthr-default.h.
  // In that case, fall back on the instantiation location.
  if (!retval) {
    retval = GetFileEntry(GetInstantiationLoc(loc));
  }
  return retval;
}

//------------------------------------------------------------
// GetFileEntry(), GetFilePath(), and GetLocation().

inline clang::SourceLocation GetLocation(const clang::Token& token) {
  return token.getLocation();
}

inline clang::SourceLocation GetLocation(clang::SourceLocation loc) {
  return loc;   // the identity location-getter, useful with templates
}
clang::SourceLocation GetLocation(const clang::Decl* decl);
clang::SourceLocation GetLocation(const clang::Stmt* stmt);
clang::SourceLocation GetLocation(const clang::TypeLoc* typeloc);
clang::SourceLocation GetLocation(const clang::NestedNameSpecifierLoc* nnsloc);
clang::SourceLocation GetLocation(const clang::TemplateArgumentLoc* argloc);

// These define default implementations of GetFileEntry() and
// GetPath() in terms of GetLocation().  As long as an object defines
// its own GetLocation(), it will get these other two for free.
template<typename T> const clang::FileEntry* GetFileEntry(const T& obj) {
  return GetFileEntry(GetLocation(obj));
}
template<typename T> const string GetFilePath(const T& obj) {
  return GetFilePath(GetFileEntry(obj));
}

//------------------------------------------------------------
// Some utility, location-based routines.

// Given any two objects that have instantiation-locations, says
// whether one occurs before the other in the translation unit (using
// instantiated locations).  This means that one would occur before
// the other looking at the output of cc -E or equivalent.
template<typename T, typename U>
inline bool IsBeforeInTranslationUnit(const T& a, const U& b) {
  const clang::FullSourceLoc a_loc(GetLocation(a), *GlobalSourceManager());
  const clang::FullSourceLoc b_loc(GetLocation(b), *GlobalSourceManager());
  // Inside a macro, everything has the same instantiation location.
  // We'd like to use spelling-location to break that tie, but it's
  // unreliable since a or b might be spelled in "<scratch space>".
  // So we're just conservative and return true always if the two have
  // an equal location and are in a macro.  (Because we check the
  // instantiation-location is equal, it's enough that one of the two
  // be in a macro; we prefer that since IsInMacro fails if T or U is
  // the wrong type.)  TODO(csilvers): see if's possible to get
  // isBeforeInTranslationUnitThan working properly.  This may require
  // storing source-locations better in OneUse.
  if ((IsInMacro(a_loc) || IsInMacro(b_loc)) &&
      GetInstantiationLoc(a_loc) == GetInstantiationLoc(b_loc))
    return true;
  return a_loc.isBeforeInTranslationUnitThan(b_loc);
}

// Like IsBeforeInTranslationUnit, but both a and b must be
// instantiated in the same file as well.
template<typename T, typename U>
inline bool IsBeforeInSameFile(const T& a, const U& b) {
  if (GetFileEntry(a) != GetFileEntry(b))
    return false;
  return IsBeforeInTranslationUnit(a, b);
}

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_FILE_UTIL_H_
