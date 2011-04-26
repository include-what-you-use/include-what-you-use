//===--- iwyu_preprocessor.cpp - handle #includes/#defines for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#include <string>
#include <utility>
#include "iwyu_ast_util.h"
#include "iwyu_globals.h"
#include "iwyu_include_picker.h"
#include "iwyu_lexer_utils.h"
#include "iwyu_location_util.h"
#include "iwyu_output.h"
#include "iwyu_path_util.h"
#include "iwyu_preprocessor.h"
#include "iwyu_stl_util.h"
#include "clang/Lex/MacroInfo.h"

using clang::FileEntry;
using clang::MacroInfo;
using clang::PresumedLoc;
using clang::SourceLocation;
using clang::SourceRange;
using clang::Token;
using llvm::errs;
using std::make_pair;
using std::string;

namespace SrcMgr = clang::SrcMgr;

// Prints to errs() if the verbose level is at a high enough level to
// print symbols that occur in the given file.  This is only valid
// when used inside a class, such as IwyuAstConsumer, that defines a
// method named ShouldPrintSymbolFromFile().
#define ERRSYM(file_entry) \
  if (!ShouldPrintSymbolFromFile(file_entry)) ; else ::llvm::errs()


namespace include_what_you_use {

namespace {
// TODO(user): Perhaps make this method accessible iwyu-wide.
// At first blush, iwyu_output is the place to put it, but that would
// introduce a circular dependency between iwyu_output and
// iwyu_ast_util.
void Warn(SourceLocation loc, const string& message) {
  errs() << PrintableLoc(loc) << ": warning: " << message << "\n";
}
}  // namespace

//------------------------------------------------------------
// Utilities for examining source files.

// For a particular #include line that include_loc points to,
// returns the include as typed by the user, including <> or "".
// This works even for computed #includes ('#include MACRO'): we
// point to the string the macro expands to.
static string GetIncludeNameAsTyped(SourceLocation include_loc) {
  return GetIncludeNameAsTyped(include_loc, DefaultDataGetter());  // iwyu_lexer
}

// For a particular #include line that include_loc points to,
// returns true if the given text occurs on the line.
// (Case sensitive.)
static bool IncludeLineHasText(SourceLocation include_loc,
                               const string& text) {
  const string data = GetSourceTextUntilEndOfLine(include_loc,
                                                  DefaultDataGetter());
  return data.find(text) != string::npos;
}

//------------------------------------------------------------
// Utilities on macros.

static string GetName(const Token& token) {
  return token.getIdentifierInfo()->getName().str();
}

//------------------------------------------------------------
// Utilities for handling iwyu-specific pragma comments.

bool IwyuPreprocessorInfo::IncludeLineIsInExportedRange(
    clang::SourceLocation includer_loc) const {
  return ContainsKey(exported_lines_set_,
                     make_pair(GetFileEntry(includer_loc),
                               GetLineNumber(includer_loc)));
}

void IwyuPreprocessorInfo::AddExportedRange(const clang::FileEntry* file,
                                            int begin_line, int end_line) {
  // Crude algorithm - add every line in the range to a set.
  for (int line_number = begin_line; line_number < end_line; ++line_number) {
    exported_lines_set_.insert(make_pair(file, line_number));
  }
}

namespace {

// Given a vector of tokens, a token to match, and an expected number
// of tokens, return true if the number of tokens is at least the
// expected number and the first token matches the given token, else
// false. In addition, if in the 'return true' case there are more
// tokens than expected, warn if the first one doesn't start "//".
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
      !StartsWith(tokens[num_expected_tokens], "//")) {
    Warn(loc, "Extra tokens on pragma line");
  }
  return true;
}

// Given a vector of tokens, two tokens to match, and an expected
// number of tokens, return true if the number of tokens is at least
// the expected number and the first two tokens match the given
// tokens, else false. In addition, if in the 'return true' case there
// are more tokens than expected, warn if the first one doesn't start
// "//".
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
      !StartsWith(tokens[num_expected_tokens], "//")) {
    // Accept but warn.
    Warn(loc, "Extra tokens on pragma line");
  }
  return true;
}

}  // namespace

// IWYU pragma processing.
void IwyuPreprocessorInfo::ProcessPragmasInFile(SourceLocation file_beginning) {
  SourceLocation current_loc = file_beginning;
  SourceLocation begin_exports_location;

  while (true) {
    current_loc = GetLocationAfter(current_loc,
                                   "// IWYU pragma: ",
                                   DefaultDataGetter());
    if (!current_loc.isValid()) {
      break;
    }

    const string pragma_text = GetSourceTextUntilEndOfLine(current_loc,
                                                           DefaultDataGetter());
    vector<string> tokens = SplitOnWhiteSpacePreservingQuotes(pragma_text, 0);
    if (begin_exports_location.isValid()) {
      if (MatchOneToken(tokens, "end_exports", 1, current_loc)) {
        AddExportedRange(GetFileEntry(file_beginning),
                         GetLineNumber(begin_exports_location) + 1,
                         GetLineNumber(current_loc));
        begin_exports_location = SourceLocation();  // Invalidate it.
      } else {
        // No pragma allowed within "begin_exports" - "end_exports"
        Warn(current_loc, "Expected end_exports pragma");
      }
      continue;
    }

    if (MatchOneToken(tokens, "begin_exports", 1, current_loc)) {
      begin_exports_location = current_loc;
      continue;
    }

    if (MatchOneToken(tokens, "end_exports", 1, current_loc)) {
      Warn(current_loc, "end_exports without a begin_exports");
      continue;
    }

    if (MatchTwoTokens(tokens, "private,", "include", 3, current_loc)) {
      // 3rd token should be a quoted header.
      const string quoted_include
          = ConvertToQuotedInclude(GetFilePath(current_loc));
      MutableGlobalIncludePicker()->AddMapping(quoted_include, tokens[2]);
      MutableGlobalIncludePicker()->MarkIncludeAsPrivate(quoted_include);
      ERRSYM(GetFileEntry(current_loc)) << "Adding private pragma-mapping: "
                                        << quoted_include << " -> "
                                        << tokens[2] << "\n";
      continue;
    }

    if (MatchOneToken(tokens, "no_include", 2, current_loc)) {
      // 2nd token should be an quoted header.
      no_include_map_[GetFileEntry(current_loc)].insert(tokens[1]);
      ERRSYM(GetFileEntry(current_loc)) << "Inhibiting include of "
                                        << tokens[1] << "\n";
      continue;
    }

    // "keep" and "export" are handled in MaybeProtectInclude.
    if (!MatchOneToken(tokens, "keep", 1, current_loc)
        && !MatchOneToken(tokens, "export", 1, current_loc)) {
      Warn(current_loc, "Unknown or malformed pragma (" + pragma_text + ")");
      continue;
    }
  }

  if (begin_exports_location.isValid()) {
    Warn(begin_exports_location,
         "begin_exports without an end_exports");
  }
}

void IwyuPreprocessorInfo::ProcessHeadernameDirectivesInFile(
    SourceLocation file_beginning) {
  SourceLocation current_loc = file_beginning;
  SourceLocation begin_exports_location;

  while (true) {
    // TODO(user): Maybe place restrictions on the
    // placement. E.g., in a comment, before any code, or perhaps only
    // when in the same comment as an @file directive.
    current_loc = GetLocationAfter(current_loc,
                                   "@headername{",
                                   DefaultDataGetter());
    if (!current_loc.isValid()) {
      break;
    }

    string after_text = GetSourceTextUntilEndOfLine(current_loc,
                                                    DefaultDataGetter());
    const string::size_type close_brace_pos = after_text.find('}');
    if (close_brace_pos == string::npos) {
      Warn(current_loc, "@headername directive missing a closing brace");
      continue;
    }
    after_text = after_text.substr(0, close_brace_pos);
    vector<string> public_includes = Split(after_text, ",", 0);

    const string quoted_private_include
        = ConvertToQuotedInclude(GetFilePath(current_loc));
    for (string::size_type i = 0; i < public_includes.size(); ++i) {
      StripWhiteSpace(&public_includes[i]);
      const string quoted_header_name = "<" + public_includes[i] + ">";
      MutableGlobalIncludePicker()->AddMapping(quoted_private_include,
                                               quoted_header_name);
      MutableGlobalIncludePicker()->MarkIncludeAsPrivate(
          quoted_private_include);
      ERRSYM(GetFileEntry(current_loc)) << "Adding @headername mapping: "
                                        << quoted_private_include << "->"
                                        << quoted_header_name << "\n";
    }
    break;  // No more than one @headername directive allowed.
  }
}

//------------------------------------------------------------
// Utilities for adding #includes.

// Helper function that returns iwyu_file_info_map_[file_entry] if
// it already exists, or creates a new one and returns it otherwise.
IwyuFileInfo* IwyuPreprocessorInfo::GetFromFileInfoMap(const FileEntry* file) {
  IwyuFileInfo* iwyu_file_info = FindInMap(&iwyu_file_info_map_, file);
  if (!iwyu_file_info) {
    iwyu_file_info_map_.insert(make_pair(file, IwyuFileInfo(file, this)));
    iwyu_file_info = FindInMap(&iwyu_file_info_map_, file);
    CHECK_(iwyu_file_info);   // should succeed this time!
  }
  return iwyu_file_info;
}

// Sometimes, we can tell just by looking at an #include line
// that iwyu should never recommend removing the #include.  For
// instance, if it has an IWYU pragma saying to keep it.
void IwyuPreprocessorInfo::MaybeProtectInclude(
    SourceLocation includer_loc, const FileEntry* includee,
    const string& include_name_as_typed) {
  const FileEntry* includer = GetFileEntry(includer_loc);
  if (IsBuiltinOrCommandLineFile(includer))
    return;

  string protect_reason;
  // We always keep lines with pragmas "keep" or "export".
  if (IncludeLineHasText(includer_loc, "// IWYU pragma: keep")) {
    protect_reason = "pragma_keep";

  } else if (IncludeLineHasText(includer_loc, "// IWYU pragma: export") ||
             IncludeLineIsInExportedRange(includer_loc)) {
    protect_reason = "pragma_export";
    const string quoted_includer = ConvertToQuotedInclude(GetFilePath(includer));
    MutableGlobalIncludePicker()->AddMapping(include_name_as_typed,
                                             quoted_includer);
    ERRSYM(includer) << "Adding pragma-export mapping: "
                     << include_name_as_typed << " -> " << quoted_includer
                     << "\n";

  // We also always keep #includes of .c files: iwyu doesn't touch those.
  // TODO(csilvers): instead of IsHeaderFile, check if the file has
  // any "non-inlined" definitions.
  } else if (!IsHeaderFile(GetFilePath(includee))) {
    protect_reason = ".cc include";

  // There's one more place we keep the #include: if our file re-exports it.
  // (A decision to re-export an #include counts as a "use" of it.)
  // But we need to finalize all #includes before we can test that,
  // so we do it in a separate function, ProtectReexportIncludes, below.

  }

  if (!protect_reason.empty()) {
    CHECK_(ContainsKey(iwyu_file_info_map_, includer));
    GetFromFileInfoMap(includer)->ReportIncludeFileUse(include_name_as_typed);
    ERRSYM(includer) << "Marked dep: " << GetFilePath(includer)
                     << " needs to keep " << include_name_as_typed
                     << " (reason: " << protect_reason << ")\n";
  }
}

static void ProtectReexportIncludes(
    map<const FileEntry*, IwyuFileInfo>* file_info_map) {
  for (map<const FileEntry*, IwyuFileInfo>::iterator
           it = file_info_map->begin(); it != file_info_map->end(); ++it) {
    IwyuFileInfo& includer = it->second;
    set<const FileEntry*> incs = includer.direct_includes_as_fileentries();
    const string includer_path = GetFilePath(it->first);
    for (Each<const FileEntry*> include(&incs); !include.AtEnd(); ++include) {
      const string includee_path = GetFilePath(*include);
      if (GlobalIncludePicker().HasMapping(includee_path, includer_path)) {
        includer.ReportIncludeFileUse(ConvertToQuotedInclude(includee_path));
        ERRSYM(it->first) << "Marked dep: " << includer_path << " needs to keep"
                          << " " << includee_path << " (reason: re-exports)\n";
      }
    }
  }
}


// Called when a #include is encountered.  i_n_a_t includes <> or "".
// We keep track of this information in two places:
// 1) iwyu_file_info_map_ maps the includer as a FileEntry* to the
//    includee both as the literal name used and as a FileEntry*.
// 2) include_to_fileentry_map_ maps the includee's literal name
//    as typed to the FileEntry* used.  This can be used (in a
//    limited way, due to non-uniqueness concerns) to map between
//    names and FileEntries.
// We also tell this #include to the include-picker, which may
// use it to fine-tune its include-picking algorithms.
void IwyuPreprocessorInfo::AddDirectInclude(
    SourceLocation includer_loc, const FileEntry* includee,
    const string& include_name_as_typed) {
  if (IsBuiltinOrCommandLineFile(includee))
    return;

  // For files we're going to be reporting IWYU errors for, we need
  // both forms of the includee to be specified.  For other files, we
  // don't care as much.
  const FileEntry* includer = GetFileEntry(includer_loc);
  if (ShouldReportIWYUViolationsFor(includer)) {
    CHECK_(includee != NULL);
    CHECK_(!include_name_as_typed.empty());
  }

  GetFromFileInfoMap(includer)->AddInclude(
      includee, include_name_as_typed, GetLineNumber(includer_loc));
  // Make sure the includee has a file-info-map entry too.
  (void)GetFromFileInfoMap(includee);

  // We have a rule that if foo.h #includes bar.h, foo.cc doesn't need
  // to #include bar.h as well, but instead gets it 'automatically' via
  // foo.h.  We say that 'foo.h' is an "internal header" for foo.cc.
  if (includer == main_file_ && BelongsToMainCompilationUnit(includee))
    GetFromFileInfoMap(includer)->AddInternalHeader(
        GetFromFileInfoMap(includee));

  // Also keep track of what FileEntry we ended up using for this name.
  // Because we use #include-next, the same include-name can map to
  // several files; we use the first such mapping we see, which is the
  // top of the #include-next chain.
  if (!include_name_as_typed.empty()) {
    if (!ContainsKey(include_to_fileentry_map_, include_name_as_typed)) {
      include_to_fileentry_map_[include_name_as_typed] = includee;
    }
  }

  // Tell the include-picker about this new include.
  MutableGlobalIncludePicker()->AddDirectInclude(GetFilePath(includer),
                                                 GetFilePath(includee));

  MaybeProtectInclude(includer_loc, includee, include_name_as_typed);

  ERRSYM(includer) << "Added an #include: " << GetFilePath(includer)
                   << " -> " << include_name_as_typed << "\n";
}

//------------------------------------------------------------
// Preprocessor event handlers.

// Called whenever a macro is expanded.  Example: when FOO(a, b) is
// seen in the source code, where FOO() is a macro #defined earlier,
// MacroExpands() will be called once with 'macro_use_token' being
// FOO, and 'macro_def' containing more information about FOO's
// definition.
void IwyuPreprocessorInfo::MacroExpands(const Token& macro_use_token,
                                        const MacroInfo* macro_def) {
  const FileEntry* macro_file = GetFileEntry(macro_use_token);
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
                                        const MacroInfo* macro) {
  const SourceLocation macro_loc = macro->getDefinitionLoc();
  ERRSYM(GetFileEntry(macro_loc))
      << "[ #define     ] " << PrintableLoc(macro_loc)
      << ": " << GetName(id) << "\n";
  // We'd like to do an iwyu check on every token in the macro
  // definition, but without knowing how and where the macro will be
  // used, we don't have enough context to.  But we *can* check those
  // tokens that are macro calls: that is, one macro calling another.
  // We can't do the checking as we go, since macros can refer to
  // macros that come after them in the source file, so we just store
  // every macro that's defined, and every macro it calls from its
  // body, and then after reading the whole file we do an iwyu
  // analysis on the results.  (This can make mistakes if the code
  // #undefs and re-defines a macro, but should work fine in practice.)
  if (macro_loc.isValid())
    macros_definition_loc_[GetName(id)] = macro_loc;
  for (MacroInfo::tokens_iterator it = macro->tokens_begin();
       it != macro->tokens_end(); ++it) {
    const Token& token_in_macro = *it;
    if (token_in_macro.getKind() == clang::tok::identifier &&
        token_in_macro.getIdentifierInfo()->hasMacroDefinition()) {
      macros_called_from_macros_.push_back(token_in_macro);
    }
  }
}

void IwyuPreprocessorInfo::If(SourceRange range) {
  ERRSYM(GetFileEntry(range.getBegin()))
      << " [ #if         ] "
      << PrintableSourceRange(range) << "\n";
  CheckIfOrElif(range);
}

void IwyuPreprocessorInfo::Elif(SourceRange range) {
  ERRSYM(GetFileEntry(range.getBegin()))
      << " [ #elif       ] "
      << PrintableSourceRange(range) << "\n";
  CheckIfOrElif(range);
}

void IwyuPreprocessorInfo::Ifdef(const Token& id) {
  ERRSYM(GetFileEntry(id.getLocation()))
      << "[ #ifdef      ] " << PrintableLoc(id.getLocation())
      << ": " << GetName(id) << "\n";
  FindAndReportMacroUse(GetName(id), id.getLocation());
}

void IwyuPreprocessorInfo::Ifndef(const Token& id) {
  ERRSYM(GetFileEntry(id.getLocation()))
      << "[ #ifndef     ] " << PrintableLoc(id.getLocation())
      << ": " << GetName(id) << "\n";
  FindAndReportMacroUse(GetName(id), id.getLocation());
}

void IwyuPreprocessorInfo::FileChanged(SourceLocation loc,
                                       FileChangeReason reason,
                                       SrcMgr::CharacteristicKind file_type) {
  switch (reason) {
    case EnterFile:
      FileChanged_EnterFile(loc);
      break;
    case ExitFile:
      FileChanged_ExitToFile(loc);
      break;
    case RenameFile:
      FileChanged_RenameFile(loc);
      break;
    case SystemHeaderPragma:
      // We see "#pragma GCC system_header".
      FileChanged_SystemHeaderPragma(loc);
      break;
    default:
      CHECK_(!"Unknown file change reason");
  }
}

// Called when we see an #include, but decide we don't need to
// actually read it because it's already been #included (and is
// protected by a header guard).
void IwyuPreprocessorInfo::FileSkipped(const FileEntry& file,
                                       const Token &filename,
                                       SrcMgr::CharacteristicKind file_type) {
  const SourceLocation include_loc = filename.getLocation();
  const string include_name_as_typed = GetIncludeNameAsTyped(include_loc);
  ERRSYM(GetFileEntry(include_loc))
      << "[ (#include)  ] " << include_name_as_typed
      << " (" << GetFilePath(&file) << ")\n";

  AddDirectInclude(include_loc, &file, include_name_as_typed);
}

// Called when a file is #included.
void IwyuPreprocessorInfo::FileChanged_EnterFile(
    SourceLocation file_beginning) {
  // Get the location of the #include directive that resulted in the
  // include of the file that file_beginning is in.
  const PresumedLoc presumed =
      GlobalSourceManager()->getPresumedLoc(file_beginning);
  const SourceLocation include_loc = presumed.getIncludeLoc();
  string include_name_as_typed = "";
  if (!IsBuiltinOrCommandLineFile(GetFileEntry(include_loc))) {
    include_name_as_typed = GetIncludeNameAsTyped(include_loc);
  }
  ERRSYM(GetFileEntry(include_loc))
      << "[ #include    ] " << include_name_as_typed
      << " (" << GetFilePath(file_beginning) << ")\n";

  const FileEntry* const new_file = GetFileEntry(file_beginning);
  if (new_file)
    AddDirectInclude(include_loc, new_file, include_name_as_typed);

  if (IsBuiltinOrCommandLineFile(new_file))
    return;

  ProcessPragmasInFile(file_beginning);
  ProcessHeadernameDirectivesInFile(file_beginning);

  // The first non-special file entered is the main file.
  if (main_file_ == NULL)
    main_file_ = new_file;

  if (main_file_ != NULL && BelongsToMainCompilationUnit(new_file)) {
    VERRS(5) << "Added to main compilation unit: "
             << GetFilePath(new_file) << "\n";
    AddGlobToReportIWYUViolationsFor(GetFilePath(new_file));
  }
  if (ShouldReportIWYUViolationsFor(new_file)) {
    files_to_report_iwyu_violations_for_.insert(new_file);
  }
}

// Called when done with an #included file and returning to the parent file.
void IwyuPreprocessorInfo::FileChanged_ExitToFile(SourceLocation include_loc) {
  ERRSYM(GetFileEntry(include_loc)) << "[ Exiting to  ] "
                                    << PrintableLoc(include_loc) << "\n";
}

void IwyuPreprocessorInfo::FileChanged_RenameFile(SourceLocation new_file) {
  ERRSYM(GetFileEntry(new_file)) << "[ Renaming to ] "
                                 << PrintableLoc(new_file) << "\n";
}

void IwyuPreprocessorInfo::FileChanged_SystemHeaderPragma(SourceLocation loc) {
  ERRSYM(GetFileEntry(loc)) << "[ #pragma s_h ] "
                            << PrintableLoc(loc) << "\n";
}

//------------------------------------------------------------
// Iwyu checkers.

// Checks whether it's OK to use the given macro defined in file defined_in.
void IwyuPreprocessorInfo::ReportMacroUse(const string& name,
                                          SourceLocation usage_location,
                                          SourceLocation dfn_location) {
  const FileEntry* used_in = GetFileEntry(usage_location);
  const string defined_path = GetFilePath(dfn_location);

  if (!ShouldReportIWYUViolationsFor(used_in))
    return;             // ignore symbols used outside foo.{h,cc}
  // Don't report macro uses that aren't actually in a file somewhere.
  if (!dfn_location.isValid() || GetFilePath(dfn_location) == "<built-in>")
    return;

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
  GetFromFileInfoMap(used_in)->ReportFullSymbolUse(usage_location,
                                                   defined_path, name);
}

// As above, but get the definition location from macros_definition_loc_.
void IwyuPreprocessorInfo::FindAndReportMacroUse(const string& name,
                                                 SourceLocation loc) {
  if (const SourceLocation* dfn_loc
      = FindInMap(&macros_definition_loc_, name)) {
    ReportMacroUse(name, loc, *dfn_loc);
  }
}

// Clang will give an OnExpandMacro() callback for all macro-tokens
// used inside an #if or #elif, *except* macro-tokens used within a
// 'define': for '#if FOO || defined(BAR)', clang calls
// OnExpandMacro() for FOO, but not for BAR (since macros within
// defined() aren't expanded).  We catch BAR-type uses here.
void IwyuPreprocessorInfo::CheckIfOrElif(SourceRange range) {
  const vector<Token> defined_args =
      FindArgumentsToDefined(range, DefaultDataGetter());  // in iwyu_lexer.h
  for (Each<Token> it(&defined_args); !it.AtEnd(); ++it) {
    FindAndReportMacroUse(GetTokenText(*it, DefaultDataGetter()),
                          it->getLocation());
  }
}

//------------------------------------------------------------
// Post-processing functions (done after all source is read).

// Adds of includer's includes, direct or indirect, into retval.
void IwyuPreprocessorInfo::AddAllIncludesAsFileEntries(
    const FileEntry* includer, set<const FileEntry*>* retval) const {
  set<const FileEntry*> direct_incs
      = FileInfoOrEmptyFor(includer).direct_includes_as_fileentries();
  for (Each<const FileEntry*> it(&direct_incs); !it.AtEnd(); ++it) {
    if (ContainsKey(*retval, *it))  // avoid infinite recursion
      continue;
    retval->insert(*it);
    AddAllIncludesAsFileEntries(*it, retval);
  }
}

void IwyuPreprocessorInfo::PopulateIntendsToProvideMap() {
  CHECK_(intends_to_provide_map_.empty() && "Should only call this fn once");
  // Figure out which of the header files we have are public.  We'll
  // map each one to a set of all private header files that map to it.
  map<const FileEntry*, set<const FileEntry*> > private_headers_behind;
  for (Each<const FileEntry*, IwyuFileInfo> it(&iwyu_file_info_map_);
       !it.AtEnd(); ++it) {
    const FileEntry* header = it->first;
    const vector<string> public_headers_for_header =
        GlobalIncludePicker().GetCandidateHeadersForFilepath(
            GetFilePath(header));
    for (Each<string> pub(&public_headers_for_header); !pub.AtEnd(); ++pub) {
      if (const FileEntry* public_file
          = GetOrDefault(include_to_fileentry_map_, *pub, NULL)) {
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
  // TODO(csilvers): use AddInternalHeaders() to get includes here.
  for (Each<const FileEntry*, IwyuFileInfo> it(&iwyu_file_info_map_);
       !it.AtEnd(); ++it) {
    const FileEntry* file = it->first;
    intends_to_provide_map_[file].insert(file);  // Everyone provides itself!
    if (ContainsKey(private_headers_behind, file)) {  // We're a public header.
      AddAllIncludesAsFileEntries(file, &intends_to_provide_map_[file]);
    } else {
      const set<const FileEntry*>& direct_includes
          = it->second.direct_includes_as_fileentries();
      for (Each<const FileEntry*> inc(&direct_includes); !inc.AtEnd(); ++inc) {
        intends_to_provide_map_[file].insert(*inc);
        if (ContainsKey(private_headers_behind, *inc))
          AddAllIncludesAsFileEntries(*inc, &intends_to_provide_map_[file]);
      }
    }
  }
  // Ugh, we can have two files with the same name, using
  // #include-next (e.g. /usr/include/c++/vector and
  // third_party/gcc3/vector).  Merge them.
  for (Each<const FileEntry*, IwyuFileInfo> it(&iwyu_file_info_map_);
       !it.AtEnd(); ++it) {
    const FileEntry* file = it->first;
    // See if a round-trip to string and back ends up at a different file.
    const string quoted_include = ConvertToQuotedInclude(GetFilePath(file));
    const FileEntry* other_file
        = GetOrDefault(include_to_fileentry_map_, quoted_include, file);
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
  for (Each<const FileEntry*, set<const FileEntry*> >
           it(&private_headers_behind); !it.AtEnd(); ++it) {
    const FileEntry* public_header = it->first;
    for (Each<const FileEntry*> private_header_it(&it->second);
         !private_header_it.AtEnd(); ++private_header_it) {
      const FileEntry* private_header = *private_header_it;
      CHECK_(ContainsKey(intends_to_provide_map_, private_header));
      InsertAllInto(intends_to_provide_map_[public_header],
                    &intends_to_provide_map_[private_header]);
    }
  }
  // Show our work, at a high enough verbosity level.
  for (Each<const FileEntry*, set<const FileEntry*> >
           it(&intends_to_provide_map_); !it.AtEnd(); ++it) {
    VERRS(4) << "Intends-to-provide for " << GetFilePath(it->first) << ":\n";
    for (Each<const FileEntry*> it2(&it->second); !it2.AtEnd(); ++it2) {
      VERRS(4) << "   " << GetFilePath(*it2) << "\n";
    }
  }
}

void IwyuPreprocessorInfo::PopulateTransitiveIncludeMap() {
  CHECK_(transitive_include_map_.empty() && "Should only call this fn once");
  for (Each<const FileEntry*, IwyuFileInfo> it(&iwyu_file_info_map_);
       !it.AtEnd(); ++it) {
    const FileEntry* file = it->first;
    transitive_include_map_[file].insert(file);   // everyone includes itself!
    AddAllIncludesAsFileEntries(file, &transitive_include_map_[file]);
  }
}

//------------------------------------------------------------
// The public API.

void IwyuPreprocessorInfo::HandlePreprocessingDone() {
  // In some cases, macros can refer to macros in files that are
  // defined later in other files.  In those cases, we can't
  // do an iwyu check until all header files have been read.
  // (For instance, if we see '#define FOO(x) BAR(!x)', BAR doesn't
  // actually have to be defined until FOO is actually used, which
  // could be later in the preprocessing.)
  for (Each<Token> it(&macros_called_from_macros_); !it.AtEnd(); ++it) {
    FindAndReportMacroUse(GetName(*it), it->getLocation());
  }

  // Other post-processing steps.
  MutableGlobalIncludePicker()->FinalizeAddedIncludes();
  ProtectReexportIncludes(&iwyu_file_info_map_);
  PopulateIntendsToProvideMap();
  PopulateTransitiveIncludeMap();
}

bool IwyuPreprocessorInfo::BelongsToMainCompilationUnit(const FileEntry* file)
    const {
  return file && (GetCanonicalName(GetFilePath(file)) ==
                  GetCanonicalName(GetFilePath(main_file_)));
}

const FileEntry* IwyuPreprocessorInfo::IncludeToFileEntry(
    const string quoted_include) const {
  return GetOrDefault(include_to_fileentry_map_, quoted_include, NULL);
}

IwyuFileInfo* IwyuPreprocessorInfo::FileInfoFor(const FileEntry* file) const {
  return const_cast<IwyuFileInfo*>(FindInMap(&iwyu_file_info_map_, file));
}

const IwyuFileInfo& IwyuPreprocessorInfo::FileInfoOrEmptyFor(
    const FileEntry* file) const {
  const IwyuFileInfo* retval = FindInMap(&iwyu_file_info_map_, file);
  if (retval)
    return *retval;

  return empty_file_info_;
}

bool IwyuPreprocessorInfo::PublicHeaderIntendsToProvide(
    const FileEntry* public_header, const FileEntry* other_file) const {
  if (const set<const FileEntry*>* provides
      = FindInMap(&intends_to_provide_map_, public_header)) {
    return ContainsKey(*provides, other_file);
  }
  return false;
}

bool IwyuPreprocessorInfo::FileTransitivelyIncludes(
    const FileEntry* includer, const FileEntry* includee) const {
  if (const set<const FileEntry*>* all_includes
      = FindInMap(&transitive_include_map_, includer)) {
    return ContainsKey(*all_includes, includee);
  }
  return false;
}

bool IwyuPreprocessorInfo::FileTransitivelyIncludes(
    const FileEntry* includer, const string& quoted_includee) const {
  if (const set<const FileEntry*>* all_includes
      = FindInMap(&transitive_include_map_, includer)) {
    for (Each<const FileEntry*> it(all_includes); !it.AtEnd(); ++it) {
      if (ConvertToQuotedInclude(GetFilePath(*it)) == quoted_includee)
        return true;
    }
  }
  return false;
}

bool IwyuPreprocessorInfo::FileTransitivelyIncludes(
    const string& quoted_includer, const FileEntry* includee) const {
  for (Each<const FileEntry*, set<const FileEntry*> >
           it(&transitive_include_map_); !it.AtEnd(); ++it) {
    if (ConvertToQuotedInclude(GetFilePath(it->first)) == quoted_includer)
      return ContainsKey(it->second, includee);
  }
  return false;
}

bool IwyuPreprocessorInfo::IncludeIsInhibited(
    const clang::FileEntry* file, const string& other_filename) const {
  const set<string>* inhibited_includes = FindInMap(&no_include_map_, file);
  return (inhibited_includes != NULL) &&
      ContainsKey(*inhibited_includes, other_filename);
}

}  // namespace include_what_you_use
