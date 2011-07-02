//===--- iwyu_preprocessor.h - handle #includes/#defines for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The class that gets preprocessor callbacks: #includes, #defines,
// #pragmas, and the like.
//
// It has three main tasks in iwyu:
//
// 1) Record the #include hierarchy.  For each file it sees, it creates
// an IwyuOutput class for that file, which is set up to hold the direct
// includes that the file has.
//
// 2) Record iwyu violations for macro uses.  In particular, if a macro
// is #defined in one file and the token is accessed in another, do an
// iwyu check on that use.
//

// 3) Process iwyu pragma-like constructs.  Comments beginning "//"
// are allowed to follow any pragma, otherwise extraneous text on the
// line will result in an error message being logged. Here are the
// constructs we look for:
// Full-line constructs:
//    a) // IWYU pragma: private, include "foo/bar/baz.h"
//    b) // IWYU pragma: private
//    c) // IWYU pragma: begin_exports
//    d) // IWYU pragma: end_exports
//    e) // IWYU pragma: no_include "foo/bar/baz.h"
//    f) // IWYU pragma: friend <regexp>
//       // IWYU pragma: friend "<regexp>" -- needed if spaces in regexp.
// 'Annotation' constructs:
//    d) #include "foo/bar/baz.h"  // IWYU pragma: export
//    e) #include "foo/bar/baz.h"  // IWYU pragma: keep
//
// 4) Process doxygen @headername directives. In later versions of GCC,
//    these directives are like IWYU pragma private directives:
//    @headername{foo} means to include <foo> instead.
//    The arguments are allowed to be a comma-separated list.
//    See
//    http://gcc.gnu.org/onlinedocs/libstdc++/manual/documentation_hacking.html
//
// This class finishes its processing before the 'main' iwyu
// processing is done, so other iwyu consumers can access the main
// outputs of this class:
//    * The map from include-name to FileEntry*.
//    * The map from FileEntry* to its IwyuFileInfo object.
//    * TODO(csilvers): Information about direct includes of a FileEntry*
//    * The 'intends to provide' map, which encapsulates some
//      of the information about public vs private headers.
//    * Testing and reporting membership in the main compilation unit.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_PREPROCESSOR_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_PREPROCESSOR_H_

#include <map>                          // for map
#include <set>                          // for set
#include <stack>                        // for stack
#include <string>                       // for string
#include <utility>                      // for pair
#include <vector>                       // for vector

#include "iwyu_output.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Token.h"

namespace clang {
class FileEntry;
class MacroInfo;
}

namespace include_what_you_use {

using std::map;
using std::pair;
using std::set;
using std::stack;
using std::string;
using std::vector;


class IwyuPreprocessorInfo : public clang::PPCallbacks,
                             public clang::CommentHandler {
 public:
  IwyuPreprocessorInfo() : main_file_(NULL),
                           empty_file_info_(NULL, this),
                           current_file_(NULL) {}

  // The client *must* call this from the beginning of HandleTranslationUnit()
  void HandlePreprocessingDone();

  // More direct ways of getting at this information
  const clang::FileEntry* main_file() const {
    return main_file_;
  }
  const set<const clang::FileEntry*>* files_to_report_iwyu_violations_for()
      const {
    return &files_to_report_iwyu_violations_for_;
  }

  // Given a quoted include like '<vector>', or '"ads/base.h"',
  // returns the FileEntry for that file, or NULL if none is
  // found.  If multiple files are included under the same
  // quoted-include name (which can happen via #include-next),
  // one is returned arbitrarily.  (But always the same one.)
  const clang::FileEntry* IncludeToFileEntry(const string quoted_include) const;

  // Returns an IwyuFileInfo struct (from iwyu_output.h) corresponding
  // to the given file, or NULL if no such struct can be found.
  // Note this is a const method that returns a non-const pointer.
  // Be careful if using this method in threaded contexts.
  IwyuFileInfo* FileInfoFor(const clang::FileEntry* file) const;

  // Instead of returning NULL if no file info can be found, returns
  // an empty IwyuFileInfo struct.
  const IwyuFileInfo& FileInfoOrEmptyFor(const clang::FileEntry* file) const;

  // For every file we've seen (that is, that we've #included),
  // returns what files it 'intends' to provide full type information
  // for.  The motivation is that a file like <vector> #includes
  // <memory> and doesn't expect you to have to, even though
  // technically it's required whenever you create a vector<Foo>,
  // which is really vector<Foo, alloc<Foo> >.  We say you don't have
  // to #include <memory> because vector intends to provide the full
  // types from <memory> for you.
  //    The rule we use is every file intends to provide full type
  // information for the files it directly includes.  For public
  // header files -- ones that the include-picker can map another file
  // to -- we relax the rule to say the public header intends to
  // provide *every* header file behind it, direct or no.  This scheme
  // isn't perfect -- it says <map> intends to provide the full type
  // of pair<>, when really it just uses it internally -- but it's a
  // reasonable heuristic.
  //    Returns true iff our analysis shows that public_header intends
  // to provide all the symbols in other_file.
  bool PublicHeaderIntendsToProvide(const clang::FileEntry* public_header,
                                    const clang::FileEntry* other_file) const;

  // Returns true if the first file directly or indirectly includes
  // the second.
  bool FileTransitivelyIncludes(const clang::FileEntry* includer,
                                const clang::FileEntry* includee) const;
  bool FileTransitivelyIncludes(const clang::FileEntry* includer,
                                const string& quoted_includee) const;
  // This seems like a weird way to call this function, but we
  // happen to need this in iwyu_output.cc:
  bool FileTransitivelyIncludes(const string& quoted_includer,
                                const clang::FileEntry* includee) const;

  // Return true if the given file has an iwyu_pragma
  // "no_include <other_filename>".
  bool IncludeIsInhibited(const clang::FileEntry* file,
                          const string& other_filename) const;

 protected:
  // Preprocessor event handlers called by Clang.
  virtual void MacroExpands(const clang::Token& id,
                            const clang::MacroInfo* macro);
  virtual void MacroDefined(const clang::Token& id,
                            const clang::MacroInfo* macro);
  // Not needed for iwyu:
  // virtual void MacroUndefined(const clang::Token&, const clang::MacroInfo*);

  virtual void If(clang::SourceRange range);
  virtual void Elif(clang::SourceRange range);
  virtual void Ifdef(const clang::Token& id);
  virtual void Ifndef(const clang::Token& id);
  // Not needed for iwyu:
  // virtual void Else();
  // virtual void Endif();

  virtual void FileChanged(clang::SourceLocation loc, FileChangeReason reason,
                           clang::SrcMgr::CharacteristicKind file_type);
  virtual void FileSkipped(const clang::FileEntry& file,
                           const clang::Token &filename,
                           clang::SrcMgr::CharacteristicKind file_type);
  // FileChanged is actually a multi-plexer for 4 different callbacks.
  void FileChanged_EnterFile(clang::SourceLocation file_beginning);
  void FileChanged_ExitToFile(clang::SourceLocation include_loc);
  void FileChanged_RenameFile(clang::SourceLocation new_file);
  void FileChanged_SystemHeaderPragma(clang::SourceLocation loc);

  // CommentHandler callback.
  // Clang doc: The handler shall return true if it has pushed any
  // tokens to be read using e.g. EnterToken or EnterTokenStream.
  virtual bool HandleComment(clang::Preprocessor& pp,
                             clang::SourceRange comment_range);

 private:
  // Returns true if includee is considered part of the main
  // compilation unit.  We always generate warnings for violations in
  // files are part of the main compilation unit.
  bool BelongsToMainCompilationUnit(const clang::FileEntry* includer,
                                    const clang::FileEntry* includee) const;

  // Helper function that returns iwyu_file_info_map_[file_entry] if
  // it already exists, or creates a new one and returns it otherwise.
  IwyuFileInfo* GetFromFileInfoMap(const clang::FileEntry* file);

  // Helper for AddDirectInclude.  Checks if we should protect the
  // #include from iwyu removal.
  void MaybeProtectInclude(clang::SourceLocation includer_loc,
                           const clang::FileEntry* includee,
                           const string& include_name_as_typed);

  // Called whenever an #include is seen in the preprocessor output.
  void AddDirectInclude(clang::SourceLocation includer_loc,
                        const clang::FileEntry* includee,
                        const string& include_name_as_typed);

  // Report a "begin_exports"/"end_exports" pragma pair.
  // begin_line is first line, end_line is just after the last line.
  void AddExportedRange(const clang::FileEntry* file,
                        int begin_line, int end_line);

  // Determine if the comment is a pragma, and if so, process it.
  void HandlePragmaComment(clang::SourceRange comment_range);

  // Process @headername directives in a file.
  void ProcessHeadernameDirectivesInFile(clang::SourceLocation file_beginning);

  // Checks whether it's OK to use the given macro defined in file defined_in.
  void ReportMacroUse(const string& name,
                      clang::SourceLocation usage_location,
                      clang::SourceLocation dfn_location);

  // As above, but get the definition location from macros_definition_loc_.
  void FindAndReportMacroUse(const string& name, clang::SourceLocation loc);

  void CheckIfOrElif(clang::SourceRange range);

  // Final-processing routines done after all header files have been read.
  void DoFinalMacroChecks();
  // Helper for PopulateIntendsToProvideMap().
  void AddAllIncludesAsFileEntries(const clang::FileEntry* includer,
                                   set<const clang::FileEntry*>* retval) const;
  void PopulateIntendsToProvideMap();
  void PopulateTransitiveIncludeMap();

  // Return true if at the current point in the parse of the given file,
  // there is a pending "begin_exports" pragma.
  bool HasOpenBeginExports(const clang::FileEntry* file_entry) const;

  // The C++ source file passed in as an argument to the compiler (as
  // opposed to other files seen via #includes).
  const clang::FileEntry* main_file_;

  // All files that we should report iwyu violations in.  It defaults
  // to the "main compilation unit" (e.g. foo.cc, foo.h, foo-inl.h):
  // main_file_ and its associated .h and -inl.h files, if they exist.
  // But users can add to it via the --check_also flag.
  set<const clang::FileEntry*> files_to_report_iwyu_violations_for_;

  // These store macros seen, as we see them, and also macros that are
  // called from other macros.  We use this to do limited iwyu-testing
  // on macro tokens (we'd love to test macro bodies more completely
  // -- like we do template bodies -- but macros don't give us enough
  // context to know how to interpret the tokens we see, in general).
  map<string, clang::SourceLocation> macros_definition_loc_;  // key: macro name

  // This should logically be a set, but set<> needs Token::operator<
  // which we don't have.  Luckily, a vector works just as well.
  vector<clang::Token> macros_called_from_macros_;

  // This maps from the include-name as typed in the program
  // (including <>'s or ""'s) to the FileEntry we loaded for that
  // #include.
  map<string, const clang::FileEntry*> include_to_fileentry_map_;

  map<const clang::FileEntry*, IwyuFileInfo> iwyu_file_info_map_;

  // How many #include lines we've encountered from the given file.
  map<const clang::FileEntry*, int> num_includes_seen_;

  // Maps from a FileEntry* to all files that this file "intends" to
  // provide the symbols from.  For now, we say a file intentionally
  // provides a symbol if it defines it, or if any file it directly
  // #includes defines it.  However, if the header is a private header
  // -- as determined by the fact it's in the private->public header
  // map -- we relax the second requirement to allow any file directly
  // or indirectly included by the public file.  This isn't perfect,
  // but is as close as we can be to matching the intent of the author
  // of the public/private system.
  map<const clang::FileEntry*,
      set<const clang::FileEntry*> > intends_to_provide_map_;

  // Maps from a FileEntry* to all the files that this file includes,
  // either directly or indirectly.
  map<const clang::FileEntry*,
      set<const clang::FileEntry*> > transitive_include_map_;

  // Maps from a FileEntry* to the quoted names of files that its file
  // is directed *not* to include via the "no_include" pragma.
  map<const clang::FileEntry*, set<string> > no_include_map_;

  const IwyuFileInfo empty_file_info_;

  // The identity of the file currently being processed.
  const clang::FileEntry* current_file_;

  // For processing pragmas. It is the current stack of open
  // "begin_exports".  There should be at most one item in this stack
  // per file in the current inclusion chain..
  stack<clang::SourceLocation> begin_exports_location_stack_;
};

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_PREPROCESSOR_H_
