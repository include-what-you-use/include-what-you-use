//===--- iwyu_output.h - output-emitting code for include-what-you-use ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file contains routines to deal with all output emitted by
// iwyu plug-in.  This includes functions to sanitize include-files
// (though most of the underlying logic is in iwyu_sanitize_filepath),
// to sanitize symbol names, to emit desired include-lines properly,
// etc.  It also controls logging and verbosity levels.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_OUTPUT_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_OUTPUT_H_

#include <map>                          // for map
#include <set>                          // for set
#include <string>                       // for string, operator<
#include <vector>                       // for vector

#include "iwyu_globals.h"
#include "iwyu_stl_util.h"
#include "port.h"  // for CHECK_
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/SourceLocation.h"

namespace clang {
class FileEntry;
}

namespace include_what_you_use {

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;


class IwyuPreprocessorInfo;

// Returns true if we should print a message at the given verbosity level.
inline bool ShouldPrint(int verbose_level) {
  return verbose_level <= GlobalFlags().verbose;
}

// Returns true if we should print information about a symbol in the
// given file, at the current verbosity level.  For instance, at most
// normal verbosities, we don't print information about symbols in
// system header files.
bool ShouldPrintSymbolFromFile(const clang::FileEntry* file);

// VERRS(n) << blah;
// prints blah to errs() if the verbose level is >= n.
#define VERRS(verbose_level) \
  if (!::include_what_you_use::ShouldPrint( \
          verbose_level)) ; else ::llvm::errs()


// This data structure holds information about a single use.  Not all
// fields will be filled for all uses.
class OneUse {
 public:
  enum UseKind { kFullUse, kForwardDeclareUse };

  OneUse(const clang::NamedDecl* decl,
         clang::SourceLocation use_loc,
         UseKind use_kind,
         bool in_cxx_method_body,
         const char* comment);
  OneUse(const string& symbol_name,
         const string& dfn_filepath,
         clang::SourceLocation use_loc);

  const string& symbol_name() const { return symbol_name_; }
  const string& short_symbol_name() const { return short_symbol_name_; }
  const clang::NamedDecl* decl() const  { return decl_; }
  const string& decl_filepath() const { return decl_filepath_; }
  clang::SourceLocation use_loc() const { return use_loc_; }
  bool is_full_use() const { return use_kind_ == kFullUse; }
  bool in_cxx_method_body() const { return in_cxx_method_body_; }
  const string& comment() const { return comment_; }
  bool ignore_use() const { return ignore_use_; }
  bool is_iwyu_violation() const { return is_iwyu_violation_; }
  bool has_suggested_header() const { return !suggested_header_.empty(); }
  const string& suggested_header() const {
    CHECK_(has_suggested_header() && "Must assign suggested_header first");
    CHECK_(!ignore_use() && "Ignored uses have no suggested header");
    return suggested_header_;
  }

  void reset_decl(const clang::NamedDecl* decl) { decl_ = decl; }
  void set_full_use() { use_kind_ = kFullUse; }
  void set_forward_declare_use() { use_kind_ = kForwardDeclareUse; }
  void set_ignore_use() { ignore_use_ = true; }
  void set_is_iwyu_violation() { is_iwyu_violation_ = true; }
  void set_suggested_header(const string& fh) { suggested_header_ = fh; }

  string PrintableUseLoc() const;
  const vector<string>& public_headers();  // not const because we fill lazily
  bool PublicHeadersContain(const string& elt);
  bool NeedsSuggestedHeader() const;    // not true for fwd-declare uses, e.g.
  int UseLinenum() const;

 private:
  void SetPublicHeaders();         // sets based on decl_filepath_

  string symbol_name_;             // the symbol being used
  string short_symbol_name_;       // 'short' form of the symbol being used
  const clang::NamedDecl* decl_;   // decl of the symbol, if we know it
  string decl_filepath_;           // filepath where the symbol lives
  clang::SourceLocation use_loc_;  // where the symbol is used from
  UseKind use_kind_;               // kFullUse or kForwardDeclareUse
  bool in_cxx_method_body_;        // true if use is inside a C++ method body
  string comment_;                 // If not empty, append to clang warning msg
  vector<string> public_headers_;  // header to #include if dfn hdr is private
  string suggested_header_;        // header that allows us to satisfy use
  bool ignore_use_;                // set to true if use is discarded
  bool is_iwyu_violation_;         // set to false when we figure out it's not
};

class OneIncludeOrForwardDeclareLine {
 public:
  explicit OneIncludeOrForwardDeclareLine(const clang::NamedDecl* fwd_decl);
  OneIncludeOrForwardDeclareLine(const string& quoted_include, int linenum);

  const string& line() const { return line_; }
  bool IsIncludeLine() const;           // vs forward-declare line
  string LineNumberString() const;      // <startline>-<endline>
  bool is_desired() const { return is_desired_; }
  bool is_present() const { return is_present_; }
  const map<string, int>& symbol_counts() const { return symbol_counts_; }
  string quoted_include() const {
    CHECK_(IsIncludeLine() && "Must call quoted_include() on include lines");
    CHECK_(!fwd_decl_ && "quoted_include and fwd_decl are mutually exclusive");
    return quoted_include_;
  }
  const clang::NamedDecl* fwd_decl() const {
    CHECK_(!IsIncludeLine() && "Must call fwd_decl() on forward-declare lines");
    CHECK_(quoted_include_.empty() && "quoted_include and fwd_decl don't mix");
    return fwd_decl_;
  }

  void set_present() { is_present_ = true; }
  void set_desired() { is_desired_ = true; }
  void clear_desired() { is_desired_ = false; }
  void clear_line_numbers() { start_linenum_ = end_linenum_ = -1; }
  // Another symbol we're using that's defined in this file.
  void AddSymbolUse(const string& symbol_name);
  bool HasSymbolUse(const string& symbol_name) const;

 private:
  string line_;                     // '#include XXX' or 'class YYY;'
  int start_linenum_;
  int end_linenum_;
  bool is_desired_;                 // IWYU will recommend this line
  bool is_present_;                 // line was present before the IWYU run
  map<string, int> symbol_counts_;  // how many times we referenced each symbol
  // Only one of the following two is ever set a given line.
  string quoted_include_;           // the file we're including, for includes
  const clang::NamedDecl* fwd_decl_;  // or the fwd-decl we're emitting
};


// This class holds IWYU information about a single file (FileEntry)
// -- referred to, in the comments below, as "this file."  The keys to
// most of these methods are all quoted header paths, which are the
// include names as they would occur in a source file, including <> or
// "".  For instance, '<string>' or '"ads/test.h"'.
// TODO(csilvers): add unitests for this class.
class IwyuFileInfo {
 public:
  // TODO(csilvers): also take iwyufileinfos for 'associated' files (.h's).
  // And a source-manager.
  IwyuFileInfo(const clang::FileEntry* this_file,
               const IwyuPreprocessorInfo* preprocessor_info);

  // An 'internal' header is a header that this file #includes
  // (possibly indirectly) that we should treat as being logically
  // part of this file.  In particular, when computing the direct
  // includes of this file, we also include the direct includes of all
  // internal headers.  Examples: vector has bits/stl_vector.h as an
  // internal header; foo.cc has foo.h and foo-inl.h as internal
  // headers.  TODO(csilvers): name this better.
  void AddInternalHeader(const IwyuFileInfo* other);

  // Use these to register an iwyu declaration: either an #include or
  // a forward-declaration.

  void AddInclude(const clang::FileEntry* includee,
                  const string& quoted_includee, int linenumber);
  // definitely_keep_fwd_decl tells us that we should never suggest
  // the fwd-decl be removed, even if we don't see any uses of it.
  void AddForwardDeclare(const clang::NamedDecl* forward_declare_decl,
                         bool definitely_keep_fwd_decl);

  // Use these to register an iwyu 'use'.  It's preferable to indicate
  // an explicit type or decl being used, but if that's not available,
  // a symbol-name is acceptable as well.  There are two forms of each
  // registration routine, one for when we need the full symbol info
  // (via an #include), and one when forward-declaring is enough.

  void ReportFullSymbolUse(clang::SourceLocation use_loc,
                           const clang::NamedDecl* decl,
                           bool in_cxx_method_body, const char* comment);
  // This will mostly be used for macro tokens.
  void ReportFullSymbolUse(clang::SourceLocation use_loc,
                           const string& dfn_filepath,
                           const string& symbol);
  // TODO(dsturtevant): Can we determine in_cxx_method_body? Do we care?

  // We only allow forward-declaring of decls, not arbitrary symbols.
  void ReportForwardDeclareUse(clang::SourceLocation use_loc,
                               const clang::NamedDecl* decl,
                               bool in_cxx_method_body, const char* comment);

  // This is used when we see a // NOLINT comment, for instance.  It says
  // '#include this header file as-is, without any public-header mapping.'
  // Input is the include-line as desired: '<string.h>' or '"ads/foo.h"'.
  void ReportIncludeFileUse(const string& quoted_include);

  // This is used only in iwyu_preprocessor.cc.  TODO(csilvers): revamp?
  const set<const clang::FileEntry*>& direct_includes_as_fileentries() const {
    return direct_includes_as_fileentries_;
  }

  // The meat of iwyu: compare the actual includes and forward-declares
  // against the symbol uses, and report which uses are iwyu violations.
  // Reports violations on errs(), and returns the number of violations.
  int CalculateAndReportIwyuViolations();

 private:
  const set<string>& direct_includes() const { return direct_includes_; }
  const set<string>& desired_includes() const {
    CHECK_(desired_includes_have_been_calculated_ &&
           "Must calculate desired includes before calling desired_includes()");
    return desired_includes_;
  }
  set<string> AssociatedQuotedIncludes() const {
    set<string> associated_quoted_includes;
    for (Each<const IwyuFileInfo*> it(&internal_headers_); !it.AtEnd(); ++it)
      associated_quoted_includes.insert((*it)->quoted_file_);
    return associated_quoted_includes;
  }

  set<const clang::FileEntry*> AssociatedFileEntries() const {
    set<const clang::FileEntry*> associated_file_entries;
    for (Each<const IwyuFileInfo*> it(&internal_headers_); !it.AtEnd(); ++it)
      associated_file_entries.insert((*it)->file_);
    return associated_file_entries;
  }

  // Populates uses with full data, including is_iwyu_violation_.
  void CalculateIwyuViolations(vector<OneUse>* uses);
  // Uses uses to emit warning messages (at high enough verbosity).
  // Returns the number of warning messages found.
  int EmitWarningMessages(const vector<OneUse>& uses);
  // Uses uses and lines to emit the 'desired' set of #includes, and
  // diffs from the current.
  void EmitDiffs(const vector<OneIncludeOrForwardDeclareLine>& lines);

  // The constructor arguments.  file_ is 'this file'.
  const clang::FileEntry* file_;
  const IwyuPreprocessorInfo* preprocessor_info_;

  string quoted_file_;

  // internal_headers_ are the files 'associated' with this file: if
  // this file is foo.cc, internal_headers_ are the IwyuFileInfo's for
  // foo.h and foo-inl.h, if present.
  set<const IwyuFileInfo*> internal_headers_;

  // Holds all the uses that are reported.
  vector<OneUse> symbol_uses_;

  // Holds all the lines (#include and fwd-declare) that are reported.
  vector<OneIncludeOrForwardDeclareLine> lines_;

  // We also hold the line information in a few other data structures,
  // for ease of references.
  set<string> direct_includes_;      // key is the quoted include, eg '<set>'
  set<const clang::FileEntry*> direct_includes_as_fileentries_;
  set<const clang::NamedDecl*> direct_forward_declares_;

  // What we will recommend the #includes to be.
  set<string> desired_includes_;
  bool desired_includes_have_been_calculated_;
};


// Helpers for testing.

namespace internal {

class FakeNamedDecl : public clang::NamedDecl {
 public:
  FakeNamedDecl(const string& kind_name, const string& qual_name,
                const string& decl_filepath, int decl_linenum);

  string kind_name() const { return kind_name_; }
  string qual_name() const { return qual_name_; }
  string decl_filepath() const { return decl_filepath_; }
  int decl_linenum() const { return decl_linenum_; }

 private:
  string kind_name_;
  string qual_name_;
  string decl_filepath_;
  int decl_linenum_;
};

}  // namespace internal

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_OUTPUT_H_
