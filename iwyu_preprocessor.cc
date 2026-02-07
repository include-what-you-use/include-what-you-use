//===--- iwyu_preprocessor.cc - handle #includes/#defines for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_preprocessor.h"

#include <cstring>
#include <optional>
#include <string>                       // for string, basic_string, etc
#include <utility>                      // for pair, make_pair

#include "clang/AST/Decl.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/Token.h"
#include "iwyu_ast_util.h"
#include "iwyu_globals.h"
#include "iwyu_include_picker.h"
#include "iwyu_lexer_utils.h"
#include "iwyu_location_util.h"
#include "iwyu_output.h"
#include "iwyu_path_util.h"
#include "iwyu_port.h"  // for CHECK_
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"
#include "llvm/ADT/StringRef.h"

// TODO: Clean out pragmas as IWYU improves.
// IWYU pragma: no_include "clang/Basic/CustomizableOptional.h"

using clang::CharSourceRange;
using clang::FileEntryRef;
using clang::FileID;
using clang::IdentifierInfo;
using clang::MacroArgs;
using clang::MacroDefinition;
using clang::MacroDirective;
using clang::MacroInfo;
using clang::Module;
using clang::NamedDecl;
using clang::OptionalFileEntryRef;
using clang::Preprocessor;
using clang::SourceLocation;
using clang::SourceRange;
using clang::SrcMgr::CharacteristicKind;
using clang::Token;
using llvm::StringRef;
using llvm::errs;
using std::make_pair;
using std::string;

namespace include_what_you_use {

namespace {
// TODO(dsturtevant): Perhaps make this method accessible iwyu-wide.
// At first blush, iwyu_output is the place to put it, but that would
// introduce a circular dependency between iwyu_output and
// iwyu_ast_util.
void Warn(SourceLocation loc, const string& message) {
  errs() << PrintableLoc(loc) << ": warning: " << message << "\n";
}

// For use with no_forward_declare. Allow people to specify forward
// declares with or without the leading "::", and don't make them use
// (anonymous namespace).
string NormalizeNamespaces(string symbol) {
  if (StartsWith(symbol, "::")) {
    symbol = symbol.substr(2);
  }
  const char kAnonymousNamespaceQualifier[] = "(anonymous namespace)::";
  for (;;) {
    const string::size_type index = symbol.find(kAnonymousNamespaceQualifier);
    if (index == string::npos) {
      break;
    }
    symbol = (symbol.substr(0, index) +
              symbol.substr(index + strlen(kAnonymousNamespaceQualifier)));
  }
  return symbol;
}

}  // namespace

//------------------------------------------------------------
// Utilities for examining source files.

// For a particular #include line that include_loc points to,
// returns the include as written by the user, including <> or "".
// This works even for computed #includes ('#include MACRO'): we
// point to the string the macro expands to.
// Simplifying wrapper around the iwyu_lexer function.
static string GetIncludeNameAsWritten(SourceLocation include_loc) {
  return GetIncludeNameAsWritten(include_loc, DefaultDataGetter());
}

//------------------------------------------------------------
// Utilities on macros.

static string GetName(const Token& token) {
  return token.getIdentifierInfo()->getName().str();
}

static SourceLocation GetMacroDefLoc(const MacroInfo* macro) {
  if (!macro) {
    return SourceLocation();
  }
  return macro->getDefinitionLoc();
}

static SourceLocation GetMacroDefLoc(const MacroDefinition& macro) {
  return GetMacroDefLoc(macro.getMacroInfo());
}

//------------------------------------------------------------
// Utilities for handling iwyu-specific pragma comments.

namespace {

// Given a vector of tokens, a token to match, and an expected number
// of tokens, return true if the number of tokens is at least the
// expected number and the first token matches the given token, else
// false. In addition, if in the 'return true' case there are more
// tokens than expected, warn if the first one doesn't start "//" or
// "*/", the latter presumably closing a C-style comment.
// <loc>, which is only used for a warning message, should refer
// to the beginning of the comment containing the tokens.
bool MatchOneToken(const vector<string>& tokens,
                   const string& token,
                   size_t num_expected_tokens,
                   SourceLocation loc) {
  if (tokens.size() < num_expected_tokens) {
    return false;
  }
  if (tokens[0] != token) {
    return false;
  }
  if (tokens.size() > num_expected_tokens &&
      !StartsWith(tokens[num_expected_tokens], "//") &&
      !StartsWith(tokens[num_expected_tokens], "*/")) {
    VERRS(4) << PrintableLoc(loc) << ": warning: Extra tokens on pragma line\n";
  }
  return true;
}

// Given a vector of tokens, two tokens to match, and an expected
// number of tokens, return true if the number of tokens is at least
// the expected number and the first two tokens match the given
// tokens, else false. In addition, if in the 'return true' case there
// are more tokens than expected, warn if the first one doesn't start
// "//".
// <loc>, which is only used for a warning message, should refer
// to the beginning of the comment containing the tokens.
bool MatchTwoTokens(const vector<string>& tokens,
                    const string& token1,
                    const string& token2,
                    size_t num_expected_tokens,
                    SourceLocation loc) {
  if (tokens.size() < num_expected_tokens) {
    return false;
  }
  if (tokens[0] != token1) {
    return false;
  }
  if (tokens[1] != token2) {
    return false;
  }
  if (tokens.size() > num_expected_tokens &&
      !StartsWith(tokens[num_expected_tokens], "//") &&
      !StartsWith(tokens[num_expected_tokens], "*/")) {
    // Accept but warn.
    VERRS(4) << PrintableLoc(loc) << ": warning: Extra tokens on pragma line\n";
  }
  return true;
}

}  // namespace

// Call this function only when the given file is the one currently
// being processed (or a file directly including it, when the current
// file has not processed any comments yet). Return true if only if
// there is an open begin_exports pragma in the current state of the
// parse of the given file. Note that there may be open begin_exports
// in including files. They don't matter for this function.
bool IwyuPreprocessorInfo::HasOpenBeginExports(
    OptionalFileEntryRef file) const {
  return !begin_exports_location_stack_.empty() &&
         GetFileEntry(begin_exports_location_stack_.top()) == file;
}

// Only call this when the given files is the one being processed
// Only return true if there is an open begin_keep pragma in the current
// state of the parse of the given file.
bool IwyuPreprocessorInfo::HasOpenBeginKeep(OptionalFileEntryRef file) const {
  return !begin_keep_location_stack_.empty() &&
         GetFileEntry(begin_keep_location_stack_.top()) == file;
}

bool IwyuPreprocessorInfo::HandleComment(Preprocessor& pp,
                                         SourceRange comment_range) {
  HandlePragmaComment(comment_range);
  return false;  // No tokens pushed.
}

void IwyuPreprocessorInfo::HandlePragmaComment(SourceRange comment_range) {
  const SourceLocation begin_loc = comment_range.getBegin();
  const SourceLocation end_loc = comment_range.getEnd();
  const char* begin_text = DefaultDataGetter().GetCharacterData(begin_loc);
  const char* end_text = DefaultDataGetter().GetCharacterData(end_loc);
  string pragma_text(begin_text, end_text);
  OptionalFileEntryRef const this_file_entry = GetFileEntry(begin_loc);

  // Pragmas must start comments.
  if (!StripLeft(&pragma_text, "// IWYU pragma: ") &&
      !StripLeft(&pragma_text, "/* IWYU pragma: ")) {
    return;
  }
  const vector<string> tokens =
      SplitOnWhiteSpacePreservingQuotes(pragma_text, 0);
  if (HasOpenBeginExports(this_file_entry)) {
    if (MatchOneToken(tokens, "end_exports", 1, begin_loc)) {
      ERRSYM(this_file_entry) << "end_exports pragma seen\n";
      SourceLocation export_loc_begin = begin_exports_location_stack_.top();
      begin_exports_location_stack_.pop();
      SourceRange export_range(export_loc_begin, begin_loc);
      export_location_ranges_.insert(
          std::make_pair(this_file_entry, export_range));
    } else {
      // No pragma allowed within "begin_exports" - "end_exports"
      Warn(begin_loc, "Expected end_exports pragma");
    }
    return;
  }

  if (HasOpenBeginKeep(this_file_entry)) {
    if (MatchOneToken(tokens, "end_keep", 1, begin_loc)) {
      ERRSYM(this_file_entry) << "end_keep pragma seen\n";
      SourceLocation keep_loc_begin = begin_keep_location_stack_.top();
      begin_keep_location_stack_.pop();
      SourceRange keep_range(keep_loc_begin, begin_loc);
      keep_location_ranges_.insert(std::make_pair(this_file_entry, keep_range));
    } else {
      // No pragmas allowed within "begin_keep" - "end_keep"
      Warn(begin_loc, "Expected end_keep pragma");
    }
    return;
  }

  if (MatchOneToken(tokens, "begin_exports", 1, begin_loc)) {
    ERRSYM(this_file_entry) << "begin_exports pragma seen\n";
    begin_exports_location_stack_.push(begin_loc);
    return;
  }

  if (MatchOneToken(tokens, "end_exports", 1, begin_loc)) {
    Warn(begin_loc, "end_exports without a begin_exports");
    return;
  }

  if (MatchOneToken(tokens, "begin_keep", 1, begin_loc)) {
    ERRSYM(this_file_entry) << "begin_keep pragma seen\n";
    begin_keep_location_stack_.push(begin_loc);
    return;
  }

  if (MatchOneToken(tokens, "end_keep", 1, begin_loc)) {
    Warn(begin_loc, "end_keep without a begin_keep");
    return;
  }

  if (MatchTwoTokens(tokens, "private,", "include", 3, begin_loc)) {
    // 3rd token should be a quoted header.
    const string& suggested = tokens[2];
    if (!IsQuotedInclude(suggested)) {
      Warn(begin_loc, "Suggested include must be a quoted header");
      return;
    }

    const string quoted_this_file =
        ConvertToQuotedInclude(GetFilePath(begin_loc));

    VERRS(8) << "Adding dynamic mapping for private pragma\n";
    MutableGlobalIncludePicker()->AddMapping(quoted_this_file,
                                             MappedInclude(suggested));
    MutableGlobalIncludePicker()->MarkIncludeAsPrivate(quoted_this_file);
    return;
  }

  if (MatchOneToken(tokens, "private", 1, begin_loc)) {
    const string path_this_file = GetFilePath(begin_loc);
    MutableGlobalIncludePicker()->MarkPathAsPrivate(path_this_file);
    ERRSYM(this_file_entry) << "Adding private path: "
                            << path_this_file << "\n";
    return;
  }

  if (MatchOneToken(tokens, "no_include", 2, begin_loc)) {
    // 2nd token should be an quoted header.
    const string& inhibited = tokens[1];
    if (!IsQuotedInclude(inhibited)) {
      Warn(begin_loc, "Inhibited include must be a quoted header");
      return;
    }

    no_include_map_[this_file_entry].insert(inhibited);
    ERRSYM(this_file_entry) << "Inhibiting include of "
                            << inhibited << "\n";
    return;
  }

  if (MatchOneToken(tokens, "no_forward_declare", 2, begin_loc)) {
    // 2nd token should be the qualified name of a symbol.
    const string normalized_symbol = NormalizeNamespaces(tokens[1]);
    no_forward_declare_map_[this_file_entry].insert(normalized_symbol);
    ERRSYM(this_file_entry) << "Inhibiting forward-declare of "
                            << normalized_symbol << "\n";
    return;
  }

  if (MatchOneToken(tokens, "friend", 2, begin_loc)) {
    // 2nd token should be a regex.
    string regex = tokens[1];
    // The regex is expected to match a quoted include.  If the user
    // didn't put quotes, assume they wanted a non-system file.
    if (!IsQuotedInclude(regex))
      regex = "\"(" + regex + ")\"";
    ERRSYM(this_file_entry) << GetFilePath(begin_loc)
                            << " adding friend regex " << regex << "\n";
    MutableGlobalIncludePicker()->AddFriendRegex(
        GetFilePath(begin_loc), regex);
    return;
  }

  if (MatchOneToken(tokens, "associated", 1, begin_loc)) {
    if (associated_pragma_location_.isInvalid()) {
      associated_pragma_location_ = begin_loc;
    }
    return;
  }

  if (MatchOneToken(tokens, "always_keep", 1, begin_loc)) {
    always_keep_files_.insert(this_file_entry);
    ERRSYM(this_file_entry)
        << "Marking include " << GetFilePath(this_file_entry)
        << " as always-keep\n";
    return;
  }

  // "keep" and "export" are handled in MaybeProtectInclude.
  if (!MatchOneToken(tokens, "keep", 1, begin_loc)
      && !MatchOneToken(tokens, "export", 1, begin_loc)) {
    Warn(begin_loc, "Unknown or malformed pragma (" + pragma_text + ")");
    return;
  }
}

void IwyuPreprocessorInfo::ProcessHeadernameDirectivesInFile(
    const string& quoted_private_include, SourceLocation file_beginning) {
  if (quoted_private_include.empty()) {
    return;
  }

  SourceLocation current_loc = file_beginning;
  OptionalFileEntryRef file = GetFileEntry(current_loc);
  if (!file) {
    return;
  }

  // Make sure we have a mappable name.
  CHECK_(IsQuotedInclude(quoted_private_include))
      << ": " << quoted_private_include;

  // Quote @headername headers based on the current file's system-headerness.
  bool is_angled = IsSystemHeader(file);

  while (true) {
    // Find any headername directive after a file directive. This is a Doxygen
    // convention in libstdc++ to point users from private to public headers.
    current_loc = GetLocationAfter(current_loc, "@file", DefaultDataGetter());
    if (!current_loc.isValid()) {
      break;
    }
    current_loc = GetLocationAfter(current_loc,
                                   "@headername{",
                                   DefaultDataGetter());
    if (!current_loc.isValid()) {
      break;
    }

    string after_text = GetSourceTextUntilEndOfLine(current_loc,
                                                    DefaultDataGetter()).str();
    const string::size_type close_brace_pos = after_text.find('}');
    if (close_brace_pos == string::npos) {
      Warn(current_loc, "@headername directive missing a closing brace");
      continue;
    }
    after_text = after_text.substr(0, close_brace_pos);
    vector<string> public_includes = Split(after_text, ",", 0);

    // Generate mappings from the private to all public names.
    for (string& public_include : public_includes) {
      StripWhiteSpace(&public_include);

      // HACK: work around known inconsistency in libstdc++ headers.
      // Upstream fix proposed:
      // https://gcc.gnu.org/pipermail/libstdc++/2024-August/059430.html
      if (quoted_private_include == "<bits/cpp_type_traits.h>" &&
          public_include == "ext/type_traits") {
        public_include = "ext/type_traits.h";
      }

      // Use the same angle/quote policy as for the private file.
      const string quoted_header_name = AddQuotes(public_include, is_angled);

      VERRS(8) << "Adding dynamic mapping for @headername\n";
      MutableGlobalIncludePicker()->AddMapping(
          quoted_private_include, MappedInclude(quoted_header_name));
      MutableGlobalIncludePicker()->MarkIncludeAsPrivate(
          quoted_private_include);
    }
    break;  // No more than one @headername directive allowed.
  }
}

//------------------------------------------------------------
// Utilities for adding #includes.

// Helper function that returns iwyu_file_info_map_[file_entry] if
// it already exists, or creates a new one and returns it otherwise.
IwyuFileInfo* IwyuPreprocessorInfo::GetFromFileInfoMap(
    OptionalFileEntryRef file) {
  IwyuFileInfo* iwyu_file_info = FindInMap(&iwyu_file_info_map_, file);
  if (!iwyu_file_info) {
    const string quoted_include = ConvertToQuotedInclude(GetFilePath(file));
    iwyu_file_info_map_.insert(
        make_pair(file, IwyuFileInfo(file, this, quoted_include)));
    iwyu_file_info = FindInMap(&iwyu_file_info_map_, file);
    CHECK_(iwyu_file_info);   // should succeed this time!
  }
  return iwyu_file_info;
}

void IwyuPreprocessorInfo::InsertIntoFileInfoMap(
    OptionalFileEntryRef file, const string& quoted_include_name) {
  if (!FindInMap(&iwyu_file_info_map_, file)) {
    iwyu_file_info_map_.insert(
        make_pair(file, IwyuFileInfo(file, this, quoted_include_name)));
  }
}

// Sometimes, we can tell just by looking at an #include line
// that iwyu should never recommend removing the #include.  For
// instance, if it has an IWYU pragma saying to keep it.
void IwyuPreprocessorInfo::MaybeProtectInclude(
    SourceLocation includer_loc, OptionalFileEntryRef includee,
    const string& include_name_as_written) {
  OptionalFileEntryRef includer = GetFileEntry(includer_loc);
  if (IsSpecialFileOrStdin(includer))
    return;

  string protect_reason;
  // We always keep lines with pragmas "keep" or "export".
  // TODO(dsturtevant): As written "// // IWYU pragma: keep" is incorrectly
  // interpreted as a pragma. Maybe do "keep" and "export" pragma handling
  // in HandleComment?
  if (LineHasText(includer_loc, "// IWYU pragma: keep") ||
      LineHasText(includer_loc, "/* IWYU pragma: keep") ||
      HasOpenBeginKeep(includer)) {
    protect_reason = "pragma_keep";
    FileInfoFor(includer)->ReportKnownDesiredFile(includee);

  } else if (ShouldKeepIncludeFor(includee)) {
    // The command line version of pragma keep.
    protect_reason = "--keep";
    FileInfoFor(includer)->ReportKnownDesiredFile(includee);

  } else if (LineHasText(includer_loc, "// IWYU pragma: export") ||
             LineHasText(includer_loc, "/* IWYU pragma: export") ||
             HasOpenBeginExports(includer)) {
    protect_reason = "pragma_export";
    const string includer_path = GetFilePath(includer);
    const string quoted_includer = ConvertToQuotedInclude(includer_path);
    MappedInclude map_to(quoted_includer, includer_path);
    VERRS(8) << "Adding dynamic mapping for export pragma: "
             << "(" << GetFilePath(includee) << ") -> (" << includer_path
             << ")\n";
    MutableGlobalIncludePicker()->AddMapping(include_name_as_written, map_to);
    // Relative includes can be problematic as map keys, because they are
    // context-dependent.  Convert it to a context-free quoted include
    // (which may contain the full path to the file), and add that too.
    string map_from = ConvertToQuotedInclude(GetFilePath(includee));
    if (map_from != include_name_as_written) {
      VERRS(8) << "Adding dynamic mapping for export pragma (relative): "
               << "(" << GetFilePath(includee) << ") -> (" << includer_path
               << ")\n";
      MutableGlobalIncludePicker()->AddMapping(map_from, map_to);
    }

  // We also always keep #includes of .c files: iwyu doesn't touch those.
  // TODO(csilvers): instead of IsHeaderFile, check if the file has any
  // "non-inlined" definitions.
  } else if (!IsHeaderFile(includee)) {
    protect_reason = ".cc include";

  // If the includee is marked as pch-in-code, it can never be removed.
  } else if (FileInfoFor(includee)->is_pch_in_code()) {
    protect_reason = "pch in code";

    // There's a few more places where we want to keep the #include, but we need
    // to finalize all #includes before we can test them, so we do it in a
    // separate function, FinalizeProtectedIncludes, below.
  }

  if (!protect_reason.empty()) {
    CHECK_(ContainsKey(iwyu_file_info_map_, includer));
    GetFromFileInfoMap(includer)->ReportIncludeFileUse(
        includee, include_name_as_written, includer_loc);
    ERRSYM(includer) << "Marked dep: " << GetFilePath(includer)
                     << " needs to keep " << include_name_as_written
                     << " (reason: " << protect_reason << ")\n";
  }
}

void IwyuPreprocessorInfo::FinalizeProtectedIncludes() {
  for (auto& entry : iwyu_file_info_map_) {
    OptionalFileEntryRef includer_file = entry.first;
    IwyuFileInfo& includer = entry.second;
    const string includer_path = GetFilePath(includer_file);

    for (OptionalFileEntryRef include :
         includer.direct_includes_as_fileentries()) {
      const string includee_path = GetFilePath(include);
      if (GlobalIncludePicker().HasMapping(includee_path, includer_path)) {
        // If includer re-exports includee, that counts as a "use" of it, so
        // protect the include.
        includer.ReportIncludeFileUse(include,
                                      ConvertToQuotedInclude(includee_path));
        ERRSYM(includer_file)
            << "Marked dep: " << includer_path << " needs to keep "
            << includee_path << " (reason: re-exports)\n";
      } else if (always_keep_files_.count(include) > 0) {
        // Includee itself contains an "always_keep" pragma that protects it
        // from removal in all includers.
        includer.ReportIncludeFileUse(include,
                                      ConvertToQuotedInclude(includee_path));
        includer.ReportKnownDesiredFile(include);
        ERRSYM(includer_file)
            << "Marked dep: " << includer_path << " needs to keep "
            << includee_path << " (reason: always_keep pragma)\n";
      }
    }
  }
}

// Called when a #include is encountered.  i_n_a_t includes <> or "".
// We keep track of this information in two places:
// 1) iwyu_file_info_map_ maps the includer as a FileEntry to the
//    includee both as the literal name used and as a FileEntry.
// 2) include_to_fileentry_map_ maps the includee's literal name
//    as written to the FileEntry used.  This can be used (in a
//    limited way, due to non-uniqueness concerns) to map between
//    names and FileEntries.
// We also tell this #include to the include-picker, which may
// use it to fine-tune its include-picking algorithms.
void IwyuPreprocessorInfo::AddDirectInclude(
    SourceLocation includer_loc, OptionalFileEntryRef includee,
    const string& include_name_as_written) {
  if (IsSpecialFileOrStdin(includee))
    return;

  // For files we're going to be reporting IWYU errors for, we need
  // both forms of the includee to be specified.  For other files, we
  // don't care as much.
  OptionalFileEntryRef includer = GetFileEntry(includer_loc);
  if (ShouldReportIWYUViolationsFor(includer)) {
    CHECK_(includee != nullptr);
    CHECK_(!include_name_as_written.empty());
  }
  ++num_includes_seen_[includer];

  GetFromFileInfoMap(includer)->AddInclude(
      includee, include_name_as_written, GetLineNumber(includer_loc));
  // Make sure the includee has a file-info-map entry too.
  InsertIntoFileInfoMap(includee, include_name_as_written);

  // The first #include in every translation unit might be a precompiled header
  // and we need to mark it as such for later analysis.
  bool is_includer_main_compilation_unit = main_file_ && includer == main_file_;
  if (is_includer_main_compilation_unit && num_includes_seen_[includer] == 1) {
    CHECK_(includee && "The first #include must be an actual file.");

    // Now we know includee is the first included header file. Mark it as
    // pch-in-code if the user requested it on command-line.
    if (GlobalFlags().pch_in_code) {
      IwyuFileInfo *includee_file_info = GetFromFileInfoMap(includee);
      includee_file_info->set_pch_in_code();
      includee_file_info->set_prefix_header();
      VERRS(4) << "Marked " << GetFilePath(includee) << " as pch-in-code.\n";
    }
  }

  // We have a rule that if foo.h #includes bar.h, foo.cc doesn't need
  // to #include bar.h as well, but instead gets it 'automatically'
  // via foo.h.  We say that 'foo.h' is an "associated header" for
  // foo.cc.  Make sure we ignore self-includes, though!
  // iwyu_output.cc gets upset if a file is its own associated header.
  if (includer == main_file_ && includee != includer &&
      BelongsToMainCompilationUnit(includer, includee)) {
    GetFromFileInfoMap(includer)
        ->AddAssociatedHeader(GetFromFileInfoMap(includee));
    VERRS(4) << "Marked " << GetFilePath(includee)
             << " as associated header of " << GetFilePath(includer) << ".\n";

    // All associated headers need to be included in IWYU analysis.
    // We can only get here if IWYU is invoked with an absolute source path and
    // its associated header is included by two different path names (e.g.
    // "rel/path/assoc.h" and "assoc.h") in different files.
    //
    // TODO: This line cannot be covered with our current test framework;
    // don't forget to add a test case if we build something better in the
    // future.
    AddGlobToReportIWYUViolationsFor(GetFilePath(includee));
  }

  // Besides marking headers as "associated header" with heuristics, the user
  // can directly mark headers with the associated pragma.
  OptionalFileEntryRef associated_includer =
      GetFileEntry(associated_pragma_location_);
  if (associated_pragma_location_.isValid() &&
      associated_includer == includer) {
    GetFromFileInfoMap(includer)->AddAssociatedHeader(
        GetFromFileInfoMap(includee));
    VERRS(4) << "Marked " << GetFilePath(includee)
             << " as associated header of " << GetFilePath(includer)
             << " due to associated pragma.\n";

    AddGlobToReportIWYUViolationsFor(GetFilePath(includee));
    associated_pragma_location_ = SourceLocation();
  }

  // Also keep track of what FileEntry we ended up using for this name.
  // Because we use #include-next, the same include-name can map to
  // several files; we use the first such mapping we see, which is the
  // top of the #include-next chain.
  if (!include_name_as_written.empty()) {
    if (!ContainsKey(include_to_fileentry_map_, include_name_as_written)) {
      include_to_fileentry_map_[include_name_as_written] = includee;
    }
  }

  // Tell the include-picker about this new include.
  MutableGlobalIncludePicker()->AddDirectInclude(
      GetFilePath(includer), GetFilePath(includee), include_name_as_written);

  MaybeProtectInclude(includer_loc, includee, include_name_as_written);

  ERRSYM(includer) << "Added an #include: " << GetFilePath(includer)
                   << " -> " << include_name_as_written << "\n";
}

//------------------------------------------------------------
// Preprocessor event handlers.

// Called whenever a macro is expanded.  Example: when FOO(a, b) is
// seen in the source code, where FOO() is a macro #defined earlier,
// MacroExpands() will be called once with 'macro_use_token' being
// FOO, and 'directive' containing more information about FOO's
// definition.
void IwyuPreprocessorInfo::MacroExpands(const Token& macro_use_token,
                                        const MacroDefinition& definition,
                                        SourceRange range,
                                        const MacroArgs* /*args*/) {
  OptionalFileEntryRef macro_file = GetFileEntry(macro_use_token);
  const MacroInfo* macro_def = definition.getMacroInfo();
  if (ShouldPrintSymbolFromFile(macro_file)) {
    errs() << "[ Use macro   ] "
           << PrintableLoc(macro_use_token.getLocation())
           << ": " << GetName(macro_use_token) << " "
           << "(from " << PrintableLoc(macro_def->getDefinitionLoc()) << ")\n";
  }

  ReportMacroUse(GetName(macro_use_token),
                 macro_use_token.getLocation(),
                 macro_def->getDefinitionLoc());
}

void IwyuPreprocessorInfo::MacroDefined(const Token& id,
                                        const MacroDirective* directive) {
  OptionalFileEntryRef macro_file = GetFileEntry(id);
  if (!ShouldReportIWYUViolationsFor(macro_file)) {
    return;
  }

  ERRSYM(macro_file) << "[ #define     ] " << PrintableLoc(GetLocation(id))
                     << ": " << GetName(id) << "\n";

  // We'd love to test macro bodies more completely -- like we do template
  // bodies -- but we don't have enough context to know how to interpret the
  // tokens we see, in general). Our heuristic is: if a token inside a macro X
  // identifies a known-defined macro Y, we say that X uses Y.
  const MacroInfo* defined_macro = directive->getMacroInfo();
  MacroInfo::const_tokens_iterator tok_begin = defined_macro->tokens_begin();
  MacroInfo::const_tokens_iterator tok_end = defined_macro->tokens_end();
  for (MacroInfo::const_tokens_iterator it = tok_begin; it != tok_end; ++it) {
    // The below is an approximation of everything Clang's
    // Preprocessor::HandleIdentifier needs to be true for a token to be
    // classified as a macro expansion.
    const Token& token = *it;
    if (token.getKind() != clang::tok::identifier)
      continue;

    if (token.isExpandDisabled())
      continue;

    const MacroInfo* macro =
        preprocessor_.getMacroInfo(token.getIdentifierInfo());
    if (!macro || !macro->isEnabled())
      continue;

    // A token with an associated macro it will only be considered for macro
    // expansion if it's object-like or function-like and followed by l-paren.
    if (macro->isFunctionLike() &&
        GetNextMacroTokenKind(macro, it) != clang::tok::l_paren) {
      continue;
    }

    // If we've come this far, this must be a token referencing another macro.
    // Report it as such.
    ReportMacroUse(GetName(token), token.getLocation(), GetMacroDefLoc(macro));
  }
}

void IwyuPreprocessorInfo::Ifdef(SourceLocation loc, const Token& id,
                                 const MacroDefinition& definition) {
  ERRSYM(GetFileEntry(id.getLocation()))
      << "[ #ifdef      ] " << PrintableLoc(id.getLocation()) << ": "
      << GetName(id) << "\n";
  ReportMacroUse(GetName(id), id.getLocation(), GetMacroDefLoc(definition));
}

void IwyuPreprocessorInfo::Ifndef(SourceLocation loc, const Token& id,
                                  const MacroDefinition& definition) {
  ERRSYM(GetFileEntry(id.getLocation()))
      << "[ #ifndef     ] " << PrintableLoc(id.getLocation()) << ": "
      << GetName(id) << "\n";
  ReportMacroUse(GetName(id), id.getLocation(), GetMacroDefLoc(definition));
}

// Clang will give a MacroExpands() callback for all macro-tokens
// used inside an #if or #elif, *except* macro-tokens used within a
// 'defined' operator. They produce a Defined() callback.
void IwyuPreprocessorInfo::Defined(const Token& id,
                                   const MacroDefinition& definition,
                                   SourceRange /*range*/) {
  ERRSYM(GetFileEntry(id.getLocation()))
      << "[ #if defined ] " << PrintableLoc(id.getLocation()) << ": "
      << GetName(id) << "\n";
  ReportMacroUse(GetName(id), id.getLocation(), GetMacroDefLoc(definition));
}

void IwyuPreprocessorInfo::InclusionDirective(SourceLocation hash_loc,
                                              const Token& include_token,
                                              StringRef filename,
                                              bool is_angled,
                                              CharSourceRange filename_range,
                                              OptionalFileEntryRef file,
                                              StringRef search_path,
                                              StringRef relative_path,
                                              const Module* suggested_module,
                                              bool module_imported,
                                              CharacteristicKind file_type) {
  include_filename_loc_ = filename_range.getBegin();
}

void IwyuPreprocessorInfo::FileChanged(SourceLocation loc,
                                       FileChangeReason reason,
                                       CharacteristicKind file_type,
                                       FileID exiting_from_id) {
  switch (reason) {
    case EnterFile:
      FileChanged_EnterFile(loc);
      return;
    case ExitFile:
      FileChanged_ExitToFile(
          loc, GlobalSourceManager()->getFileEntryRefForID(exiting_from_id));
      return;
    case RenameFile:
      FileChanged_RenameFile(loc);
      return;
    case SystemHeaderPragma:
      // We see "#pragma GCC system_header".
      FileChanged_SystemHeaderPragma(loc);
      return;
  }
  CHECK_UNREACHABLE_("Unknown file change reason");
}

// Called when we see an #include, but decide we don't need to
// actually read it because it's already been #included (and is
// protected by a header guard).
void IwyuPreprocessorInfo::FileSkipped(const FileEntryRef& file,
                                       const Token& filename,
                                       CharacteristicKind file_type) {
  CHECK_(include_filename_loc_.isValid() &&
         "Must skip file only for actual inclusion directive");
  const string include_name_as_written =
      GetIncludeNameAsWritten(include_filename_loc_);
  const SourceLocation include_loc =
      GetInstantiationLoc(filename.getLocation());
  ERRSYM(GetFileEntry(include_loc))
      << "[ (#include)  ] " << include_name_as_written << " ("
      << GetFilePath(file) << ")\n";

  AddDirectInclude(include_loc, file, include_name_as_written);
  if (ShouldReportIWYUViolationsFor(file)) {
    files_to_report_iwyu_violations_for_.insert(file);
  }
}

// Called when a file is #included.
void IwyuPreprocessorInfo::FileChanged_EnterFile(
    SourceLocation file_beginning) {
  // Get the location of the #include directive that resulted in the
  // include of the file that file_beginning is in.
  const SourceLocation include_loc = GlobalSourceManager()->getIncludeLoc(
      GlobalSourceManager()->getFileID(file_beginning));
  string include_name_as_written;
  if (!IsInSpecialFile(include_loc)) {
    CHECK_(include_filename_loc_.isValid() &&
           "Include from not built-in file must have inclusion directive");
    include_name_as_written = GetIncludeNameAsWritten(include_filename_loc_);
  }
  ERRSYM(GetFileEntry(include_loc))
      << "[ #include    ] " << include_name_as_written << " ("
      << GetFilePath(file_beginning) << ")\n";

  OptionalFileEntryRef const new_file = GetFileEntry(file_beginning);
  if (new_file)
    AddDirectInclude(include_loc, new_file, include_name_as_written);

  if (IsSpecialFile(new_file))
    return;

  ProcessHeadernameDirectivesInFile(include_name_as_written, file_beginning);

  // The first non-special file entered is the main file.
  if (main_file_ == nullptr)
    main_file_ = new_file;

  if (main_file_ != nullptr &&
      BelongsToMainCompilationUnit(GetFileEntry(include_loc), new_file)) {
    VERRS(5) << "Added to main compilation unit: "
             << GetFilePath(new_file) << "\n";
    AddGlobToReportIWYUViolationsFor(GetFilePath(new_file));
  }
  if (ShouldReportIWYUViolationsFor(new_file)) {
    files_to_report_iwyu_violations_for_.insert(new_file);
  }

  // Mark is_prefix_header.
  CHECK_(new_file && "is_prefix_header is applicable to usual files only");
  IwyuFileInfo *includee_file_info = GetFromFileInfoMap(new_file);
  OptionalFileEntryRef includer_file = GetFileEntry(include_loc);
  bool is_prefix_header = false;
  if (includer_file) {
    // File included from another prefix header file is prefix header too.
    IwyuFileInfo *includer_file_info = GetFromFileInfoMap(includer_file);
    is_prefix_header = includer_file_info->is_prefix_header();
  } else {
    // Files included from command line are prefix headers, unless it's the
    // main file.
    is_prefix_header = (new_file != main_file_);
  }
  if (is_prefix_header)
    includee_file_info->set_prefix_header();
}

// Called when done with an #included file and returning to the parent file.
void IwyuPreprocessorInfo::FileChanged_ExitToFile(
    SourceLocation include_loc, OptionalFileEntryRef exiting_from) {
  ERRSYM(GetFileEntry(include_loc))
      << "[ Exiting to  ] " << PrintableLoc(include_loc) << "\n";
  if (HasOpenBeginExports(exiting_from)) {
    Warn(begin_exports_location_stack_.top(),
         "begin_exports without an end_exports");
    begin_exports_location_stack_.pop();
  }

  if (HasOpenBeginKeep(exiting_from)) {
    Warn(begin_keep_location_stack_.top(),
         "begin_keep without an end_keep");
    begin_keep_location_stack_.pop();
  }
}

void IwyuPreprocessorInfo::FileChanged_RenameFile(SourceLocation new_file) {
  ERRSYM(GetFileEntry(new_file))
      << "[ Renaming to ] " << PrintableLoc(new_file) << "\n";
}

void IwyuPreprocessorInfo::FileChanged_SystemHeaderPragma(SourceLocation loc) {
  ERRSYM(GetFileEntry(loc)) << "[ #pragma s_h ] " << PrintableLoc(loc) << "\n";
}

//------------------------------------------------------------
// Iwyu checkers.

// Checks whether it's OK to use the given macro defined in file defined_in.
void IwyuPreprocessorInfo::ReportMacroUse(const string& name,
                                          SourceLocation usage_location,
                                          SourceLocation dfn_location) {
  // Don't report macro uses that aren't actually in a file somewhere.
  if (IsInSpecialFile(dfn_location))
    return;
  OptionalFileEntryRef used_in = GetFileEntry(usage_location);
  if (ShouldReportIWYUViolationsFor(used_in)) {
    // ignore symbols used outside foo.{h,cc}

    // TODO(csilvers): this isn't really a symbol use -- it may be ok
    // that the symbol isn't defined.  For instance:
    //    foo.h: #define FOO
    //    bar.h: #ifdef FOO ... #else ... #endif
    //    baz.cc: #include "foo.h"
    //            #include "bar.h"
    //    bang.cc: #include "bar.h"
    // We don't want to say that bar.h 'uses' FOO, and thus needs to
    // #include foo.h -- adding that #include could break bang.cc.
    // I think the solution is to have a 'soft' use -- don't remove it
    // if it's there, but don't add it if it's not.  Or something.
    GetFromFileInfoMap(used_in)->ReportMacroUse(usage_location, dfn_location,
                                                name);
  }
  OptionalFileEntryRef defined_in = GetFileEntry(dfn_location);
  GetFromFileInfoMap(defined_in)->ReportDefinedMacroUse(used_in);
}

//------------------------------------------------------------
// Post-processing functions (done after all source is read).

// Adds of includer's includes, direct or indirect, into retval.
void IwyuPreprocessorInfo::AddAllIncludesAsFileEntries(
    OptionalFileEntryRef includer, set<OptionalFileEntryRef>* retval) const {
  const IwyuFileInfo* file_info = FileInfoFor(includer);
  if (!file_info)
    return;

  for (OptionalFileEntryRef include :
       file_info->direct_includes_as_fileentries()) {
    if (ContainsKey(*retval, include))  // avoid infinite recursion
      continue;
    retval->insert(include);
    AddAllIncludesAsFileEntries(include, retval);
  }
}

void IwyuPreprocessorInfo::PopulateIntendsToProvideMap() {
  CHECK_(intends_to_provide_map_.empty() && "Should only call this fn once");
  // Figure out which of the header files we have are public.  We'll
  // map each one to a set of all private header files that map to it.
  map<OptionalFileEntryRef, set<OptionalFileEntryRef>> private_headers_behind;
  for (const auto& fileinfo : iwyu_file_info_map_) {
    OptionalFileEntryRef header = fileinfo.first;
    const vector<MappedInclude> public_headers_for_header =
        GlobalIncludePicker().GetCandidateHeadersForFilepath(
            GetFilePath(header));
    for (const MappedInclude& pub : public_headers_for_header) {
      if (OptionalFileEntryRef public_file = GetOrDefault(
              include_to_fileentry_map_, pub.quoted_include, std::nullopt)) {
        CHECK_(ContainsKey(iwyu_file_info_map_, public_file));
        if (public_file != header)  // no credit for mapping to yourself :-)
          private_headers_behind[public_file].insert(header);
      }
    }
  }
  // Everyone gets to provide from their direct includes.  Public
  // headers gets to provide from *all* their includes.  Likewise,
  // when you bring in a public header (because it's one of your
  // direct includes), you bring in all its includes as well.
  // Basically, a public header is really an equivalence class of
  // itself and all its direct includes.
  // TODO(csilvers): use AddAssociatedHeaders() to get includes here.
  const IncludePicker& picker = GlobalIncludePicker();
  for (const auto& fileinfo : iwyu_file_info_map_) {
    OptionalFileEntryRef file = fileinfo.first;
    if (file == nullptr)
      continue;
    intends_to_provide_map_[file].insert(file);  // Everyone provides itself!
    if (picker.IsPublic(file)) {
      // It is assumed that every external library header (public or private)
      // provides all its transitively included headers.
      for (OptionalFileEntryRef included_file_or_self :
           transitive_include_map_.at(file)) {
        InsertAllInto(transitive_include_map_.at(included_file_or_self),
                      &intends_to_provide_map_[included_file_or_self]);
      }
    } else {
      const set<OptionalFileEntryRef>& direct_includes =
          fileinfo.second.direct_includes_as_fileentries();
      for (OptionalFileEntryRef inc : direct_includes) {
        intends_to_provide_map_[file].insert(inc);
        if (picker.IsPublic(inc))
          AddAllIncludesAsFileEntries(inc, &intends_to_provide_map_[file]);
      }
    }
  }
  // Ugh, we can have two files with the same name, using
  // #include-next.  Merge them.
  for (const auto& fileinfo : iwyu_file_info_map_) {
    OptionalFileEntryRef file = fileinfo.first;
    // See if a round-trip to string and back ends up at a different file.
    const string quoted_include = ConvertToQuotedInclude(GetFilePath(file));
    OptionalFileEntryRef other_file =
        GetOrDefault(include_to_fileentry_map_, quoted_include, file);
    if (other_file != file) {
      InsertAllInto(intends_to_provide_map_[file],
                    &intends_to_provide_map_[other_file]);
      // TODO(csilvers): this isn't enough if there are *more* than 2
      // files with the same name.
      intends_to_provide_map_[file] = intends_to_provide_map_[other_file];
    }
  }
  // Finally, for convenience, we'll say every private header file
  // intends to provide exactly what its public header files do.
  // That way we don't always have to be mapping private headers to
  // public ones before calling this function.  Since we don't know
  // exactly what public header a private header might map to (if it
  // can map to more than one), we just union them all.
  // TODO(csilvers): this can be bad: if i1.h maps to both p1.h and
  //   p2.h, and we end up picking p1.h, and we say that i1.h intends
  //   to provide symbols from p2.h, we're promising a lie.  I think
  //   this is ok as long as IntendsToProvide means 'If when expanding
  //   a templated function or class in i1.h, you see the need for
  //   symbol Foo which isn't a template argument, don't worry about
  //   it.'  Double check whether that's true.
  // TODO(bolshakov): is it still needed after making private includes providing
  // all their transitively included headers?
  for (const auto& header_map : private_headers_behind) {
    OptionalFileEntryRef public_header = header_map.first;
    for (OptionalFileEntryRef private_header : header_map.second) {
      CHECK_(ContainsKey(intends_to_provide_map_, private_header));
      InsertAllInto(intends_to_provide_map_[public_header],
                    &intends_to_provide_map_[private_header]);
    }
  }
  // Show our work, at a high enough verbosity level.
  for (const auto& header_map : intends_to_provide_map_) {
    VERRS(4) << "Intends-to-provide for " << GetFilePath(header_map.first)
             << ":\n";
    for (OptionalFileEntryRef private_header : header_map.second) {
      VERRS(4) << "   " << GetFilePath(private_header) << "\n";
    }
  }
}

void IwyuPreprocessorInfo::PopulateTransitiveIncludeMap() {
  CHECK_(transitive_include_map_.empty() && "Should only call this fn once");
  for (const auto& fileinfo : iwyu_file_info_map_) {
    OptionalFileEntryRef file = fileinfo.first;
    transitive_include_map_[file].insert(file);   // everyone includes itself!
    AddAllIncludesAsFileEntries(file, &transitive_include_map_[file]);
  }
}

//------------------------------------------------------------
// The public API.

void IwyuPreprocessorInfo::HandlePreprocessingDone() {
  CHECK_(main_file_ && "Main file should be present");
  FileChanged_ExitToFile(SourceLocation(), main_file_);

  // Other post-processing steps.
  for (auto& file_info_map_entry : iwyu_file_info_map_) {
    file_info_map_entry.second.HandlePreprocessingDone();
  }
  MutableGlobalIncludePicker()->FinalizeAddedIncludes();
  FinalizeProtectedIncludes();
  // PopulateIntendsToProvideMap uses results of PopulateTransitiveIncludeMap.
  PopulateTransitiveIncludeMap();
  PopulateIntendsToProvideMap();
}

bool IwyuPreprocessorInfo::BelongsToMainCompilationUnit(
    OptionalFileEntryRef includer, OptionalFileEntryRef includee) const {
  // TODO: Should probably have a CHECK_(main_file_), but this method is
  // currently sometimes called with a nullptr main_file_.
  if (!includee)
    return false;
  if (IsSystemHeader(includee))
    return false;
  if (GetCanonicalName(GetFilePath(includee)) ==
      GetCanonicalName(GetFilePath(main_file_)))
    return true;
  // Heuristic: if the main compilation unit's *first* include is
  // a file with the same basename, assume that it's the 'associated'
  // .h file, even if the canonical names differ.  This catches
  // cases like 'foo/x.cc' #includes 'foo/public/x.h', or
  // 'foo/mailserver/x.cc' #includes 'foo/public/x.h'.
  // In the case of pch-in-code make this the *second* include,
  // as the PCH must always be first.
  int first_include_index = GlobalFlags().pch_in_code ? 2 : 1;
  if (includer == main_file_ &&
      ContainsKeyValue(num_includes_seen_, includer, first_include_index)) {
    if (GetCanonicalName(Basename(GetFilePath(includee))) ==
        GetCanonicalName(Basename(GetFilePath(main_file_))))
      return true;
  }
  return false;
}

OptionalFileEntryRef IwyuPreprocessorInfo::IncludeToFileEntry(
    const string quoted_include) const {
  return GetOrDefault(include_to_fileentry_map_, quoted_include, std::nullopt);
}

IwyuFileInfo* IwyuPreprocessorInfo::FileInfoFor(
    OptionalFileEntryRef file) const {
  return const_cast<IwyuFileInfo*>(FindInMap(&iwyu_file_info_map_, file));
}

bool IwyuPreprocessorInfo::PublicHeaderIntendsToProvide(
    OptionalFileEntryRef public_header, OptionalFileEntryRef other_file) const {
  if (const set<OptionalFileEntryRef>* provides =
          FindInMap(&intends_to_provide_map_, public_header)) {
    return ContainsKey(*provides, other_file);
  }
  return false;
}

bool IwyuPreprocessorInfo::FileTransitivelyIncludes(
    OptionalFileEntryRef includer, OptionalFileEntryRef includee) const {
  if (const set<OptionalFileEntryRef>* all_includes =
          FindInMap(&transitive_include_map_, includer)) {
    return ContainsKey(*all_includes, includee);
  }
  return false;
}

bool IwyuPreprocessorInfo::FileTransitivelyIncludes(
    OptionalFileEntryRef includer, const string& quoted_includee) const {
  if (const set<OptionalFileEntryRef>* all_includes =
          FindInMap(&transitive_include_map_, includer)) {
    for (OptionalFileEntryRef include : *all_includes) {
      if (ConvertToQuotedInclude(GetFilePath(include)) == quoted_includee)
        return true;
    }
  }
  return false;
}

bool IwyuPreprocessorInfo::FileTransitivelyIncludes(
    const string& quoted_includer, OptionalFileEntryRef includee) const {
  for (const auto& entry : transitive_include_map_) {
    if (ConvertToQuotedInclude(GetFilePath(entry.first)) == quoted_includer)
      return ContainsKey(entry.second, includee);
  }
  return false;
}

bool IwyuPreprocessorInfo::IncludeIsInhibited(
    OptionalFileEntryRef file, const string& other_filename) const {
  const set<string>* inhibited_includes = FindInMap(&no_include_map_, file);
  return (inhibited_includes != nullptr) &&
      ContainsKey(*inhibited_includes, other_filename);
}

bool IwyuPreprocessorInfo::ForwardDeclareIsInhibited(
    OptionalFileEntryRef file, const string& qualified_symbol_name) const {
  const string normalized_symbol_name =
      NormalizeNamespaces(qualified_symbol_name);
  const set<string>* inhibited_forward_declares =
      FindInMap(&no_forward_declare_map_, file);
  return (inhibited_forward_declares != nullptr) &&
      ContainsKey(*inhibited_forward_declares, normalized_symbol_name);
}

bool IwyuPreprocessorInfo::ForwardDeclareIsMarkedKeep(
    const NamedDecl* decl) const {
  // Use end-location so that any trailing comments match only on the last line.
  SourceLocation loc = decl->getEndLoc();

  // Is the decl part of a begin_keep/end_keep block?
  OptionalFileEntryRef file = GetFileEntry(loc);
  auto keep_ranges = keep_location_ranges_.equal_range(file);
  for (auto it = keep_ranges.first; it != keep_ranges.second; ++it) {
    if (it->second.fullyContains(loc)) {
      return true;
    }
  }
  // Is the declaration itself marked with trailing comment?
  return (LineHasText(loc, "// IWYU pragma: keep") ||
          LineHasText(loc, "/* IWYU pragma: keep"));
}

bool IwyuPreprocessorInfo::ForwardDeclareIsExported(
    const NamedDecl* decl) const {
  // Use end-location so that any trailing comments match only on the last line.
  SourceLocation loc = decl->getEndLoc();
  if (!loc.isValid())
    return false;

  // Is the decl part of a begin_exports/end_exports block?
  OptionalFileEntryRef file = GetFileEntry(loc);
  auto export_ranges = export_location_ranges_.equal_range(file);
  for (auto it = export_ranges.first; it != export_ranges.second; ++it) {
    if (it->second.fullyContains(loc)) {
      return true;
    }
  }
  // Is the declaration itself marked with trailing comment?
  return (LineHasText(loc, "// IWYU pragma: export") ||
          LineHasText(loc, "/* IWYU pragma: export"));
}
}  // namespace include_what_you_use
