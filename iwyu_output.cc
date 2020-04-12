//===--- iwyu_output.cc - output-emitting code for include-what-you-use ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_output.h"

#include <algorithm>                    // for sort, find
#include <cstdio>                       // for snprintf
// TODO(wan): make sure IWYU doesn't suggest <iterator>.
#include <iterator>                     // for find
#include <map>                          // for _Rb_tree_const_iterator, etc
#include <utility>                      // for pair, make_pair, operator>
#include <vector>                       // for vector, vector<>::iterator, etc

#include "iwyu_ast_util.h"
#include "iwyu_globals.h"
#include "iwyu_include_picker.h"
#include "iwyu_location_util.h"
#include "iwyu_path_util.h"
#include "iwyu_preprocessor.h"  // IWYU pragma: keep
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"
// TODO(wan): remove this once the IWYU bug is fixed.
// IWYU pragma: no_include "foo/bar/baz.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Basic/SourceLocation.h"

namespace include_what_you_use {

using clang::ClassTemplateDecl;
using clang::ClassTemplateSpecializationDecl;
using clang::CXXMethodDecl;
using clang::CXXRecordDecl;
using clang::Decl;
using clang::DeclContext;
using clang::EnumDecl;
using clang::FileEntry;
using clang::FunctionDecl;
using clang::NamedDecl;
using clang::NamespaceDecl;
using clang::RecordDecl;
using clang::SourceLocation;
using clang::SourceRange;
using clang::TemplateDecl;
using clang::UsingDecl;
using llvm::cast;
using llvm::dyn_cast;
using llvm::errs;
using llvm::isa;
using llvm::raw_string_ostream;
using std::map;
using std::multimap;
using std::pair;
using std::sort;
using std::to_string;
using std::vector;

namespace internal {

namespace {

class OutputLine {
 public:
  OutputLine() = default;
  explicit OutputLine(const string& line)
      : line_(line) {}
  OutputLine(const string& line, const vector<string>& symbols)
      : line_(line),
        symbols_(symbols) {
    symbols_.erase(std::remove_if(symbols_.begin(), symbols_.end(),
                                  [](const string& v) { return v.empty(); }),
                   symbols_.end());
  }

  size_t line_length() const { return line_.size(); }
  bool needs_alignment() const { return !symbols_.empty(); }
  void add_prefix(const string& prefix) { line_ = prefix + line_; }
  string printable_line(size_t min_length, size_t max_length) const;

 private:
  string line_;                     // '#include XXX' or 'class YYY;'
  vector<string> symbols_;          // symbols used from included header
};

// Append a helpful 'why' comment to the include line, containing the symbols
// used from the header. Align nicely at lower verbosity levels.
string OutputLine::printable_line(size_t min_length, size_t max_length) const {
  // If there are no symbols to mention, return the line as-is.
  if (symbols_.empty())
    return line_;

  CHECK_(max_length > 0);
  --max_length;  // Spare a char for newline character.

  // Before first symbol, print '  // for ' and pad it so 'why' comments are
  // nicely aligned.
  string symbol_prefix = "  // for ";
  if (line_.length() < min_length)
    symbol_prefix.insert(0, min_length - line_.length(), ' ');

  string result = line_;
  for (string symbol : symbols_) {
    // At verbose levels 0-2, truncate output to max_length columns.
    if (!ShouldPrint(3)) {
      // Calculate number of chars remaining.
      size_t remaining = 0;
      size_t result_length = result.length() + symbol_prefix.length();
      if (result_length < max_length)
        remaining = max_length - result_length;

      // Ellipsize, and if we can't fit any fragment of the symbol, give up.
      symbol = Ellipsize(symbol, remaining);
      if (symbol.empty())
        break;
    }

    result += symbol_prefix;
    result += symbol;
    symbol_prefix = ", ";
  }

  return result;
}

// A map that effectively allows us to dynamic cast from a NamedDecl
// to a FakeNamedDecl. When a FakeNamedDecl is created, it will be
// inserted into the map with itself as the key (implicitly casted to
// a NamedDecl).
std::map<const clang::NamedDecl*, const FakeNamedDecl*>
g_fake_named_decl_map;

// Since dynamic casting is not an option, this method is provided to
// determine if a decl is actually a FakeNamedDecl.
const FakeNamedDecl* FakeNamedDeclIfItIsOne(const clang::NamedDecl* decl) {
  return GetOrDefault(g_fake_named_decl_map, decl, nullptr);
}

}  // anonymous namespace

FakeNamedDecl::FakeNamedDecl(const string& kind_name, const string& qual_name,
                             const string& decl_filepath, int decl_linenum)
    : clang::NamedDecl(clang::Decl::Record, nullptr, clang::SourceLocation(),
                       clang::DeclarationName()),
      kind_name_(kind_name),
      qual_name_(qual_name),
      decl_filepath_(decl_filepath), decl_linenum_(decl_linenum) {
  g_fake_named_decl_map[this] = this;
}

// When testing IWYU, we provide a fake object (FakeNamedDecl) that
// needs to provide its own version of NamedDecl::getKindName() and
// NamedDecl::getQualifiedNameAsString().  Unfortunately they aren't
// virtual.  Hence we define the following helpers to dispatch the
// call ourselves.

string GetKindName(const clang::TagDecl* tag_decl) {
  const clang::NamedDecl* const named_decl = tag_decl;
  if (const FakeNamedDecl* fake = FakeNamedDeclIfItIsOne(named_decl)) {
    return fake->kind_name();
  }
  return tag_decl->getKindName().str();
}

string GetQualifiedNameAsString(const clang::NamedDecl* named_decl) {
  if (const FakeNamedDecl* fake = FakeNamedDeclIfItIsOne(named_decl)) {
    return fake->qual_name();
  }
  return GetWrittenQualifiedNameAsString(named_decl);
}

// Name we put in the comments next to an #include.
string GetShortNameAsString(const clang::NamedDecl* named_decl) {
  if (const FakeNamedDecl* fake = FakeNamedDeclIfItIsOne(named_decl)) {
    return fake->qual_name();
  }

  // This is modified from NamedDecl::getQualifiedNameAsString:
  // http://clang.llvm.org/doxygen/Decl_8cpp_source.html#l00742
  const DeclContext *decl_context = named_decl->getDeclContext();
  if (decl_context->isFunctionOrMethod())
    return named_decl->getNameAsString();

  vector<const DeclContext*> contexts;
  while (decl_context && isa<NamedDecl>(decl_context)) {
    contexts.push_back(decl_context);
    decl_context = decl_context->getParent();
  };

  std::string retval;
  raw_string_ostream ostream(retval);

  for (vector<const DeclContext*>::reverse_iterator it = contexts.rbegin();
       it != contexts.rend(); ++it) {
    if (const ClassTemplateSpecializationDecl* tpl_decl = DynCastFrom(*it)) {
      ostream << tpl_decl->getName() << "<>::";
    } else if (isa<NamespaceDecl>(*it)) {
      // We don't want to include namespaces in our shortname.
    } else if (const RecordDecl *record_decl = DynCastFrom(*it)) {
      if (!record_decl->getIdentifier())
        ostream << "(anonymous " << record_decl->getKindName() << ")::";
      else
        ostream << *record_decl << "::";
    } else if (const FunctionDecl *function_decl = DynCastFrom(*it)) {
      ostream << *function_decl << "::";   // could also add in '<< "()"'
    } else if (const EnumDecl* enum_decl = DynCastFrom(*it)) {
      if (enum_decl->isScoped()) {
        ostream << *(cast<NamedDecl>(*it)) << "::";
      } else {
        // Don't add a scope prefix for old-style unscoped enums.
      }
    } else {
      ostream << *(cast<NamedDecl>(*it)) << "::";
    }
  }
  // Due to the way DeclarationNameInfo::printName() is written, this
  // will show template arguments for templated constructors and
  // destructors.  Since iwyu only shows these when they're defined in
  // a -inl.h file, I'm not going to worry about it.
  if (named_decl->getDeclName())
    ostream << *named_decl;
  else
    ostream << "(anonymous)";

  return ostream.str();
}

}  // namespace internal

// Holds information about a single full or fwd-decl use of a symbol.
OneUse::OneUse(const NamedDecl* decl, SourceLocation use_loc,
               SourceLocation decl_loc, OneUse::UseKind use_kind,
               UseFlags flags, const char* comment)
    : symbol_name_(internal::GetQualifiedNameAsString(decl)),
      short_symbol_name_(internal::GetShortNameAsString(decl)),
      decl_(decl),
      decl_loc_(GetInstantiationLoc(decl_loc)),
      decl_file_(GetFileEntry(decl_loc_)),
      decl_filepath_(GetFilePath(decl_file_)),
      use_loc_(use_loc),
      use_kind_(use_kind),             // full use or fwd-declare use
      use_flags_(flags),
      comment_(comment ? comment : ""),
      ignore_use_(false),
      is_iwyu_violation_(false) {
}

// This constructor always creates a full use.
OneUse::OneUse(const string& symbol_name, const FileEntry* dfn_file,
               const string& dfn_filepath, SourceLocation use_loc)
    : symbol_name_(symbol_name),
      short_symbol_name_(symbol_name),
      decl_(nullptr),
      decl_file_(dfn_file),
      decl_filepath_(dfn_filepath),
      use_loc_(use_loc),
      use_kind_(kFullUse),
      use_flags_(UF_None),
      ignore_use_(false),
      is_iwyu_violation_(false) {
  // Sometimes dfn_filepath is actually a fully quoted include.  In
  // that case, we take that as an unchangable mapping that we
  // should never remove, so we make it the suggested header.
  CHECK_(!decl_filepath_.empty() && "Must pass a real filepath to OneUse");
  if (decl_filepath_[0] == '"' || decl_filepath_[0] == '<')
    suggested_header_ = decl_filepath_;
}

void OneUse::reset_decl(const clang::NamedDecl* decl) {
    CHECK_(decl_ && "Need existing decl to reset it");
    CHECK_(decl && "Need to reset decl with existing decl");
    decl_ = decl;
    decl_file_ = GetFileEntry(decl);
    decl_filepath_ = GetFilePath(decl);
}

int OneUse::UseLinenum() const {
  return GetLineNumber(use_loc_);
}

string OneUse::PrintableUseLoc() const {
  return PrintableLoc(use_loc());
}

void OneUse::SetPublicHeaders() {
  // We should never need to deal with public headers if we already know
  // who we map to.
  CHECK_(suggested_header_.empty() && "Should not need a public header here");
  const IncludePicker& picker = GlobalIncludePicker();   // short alias
  const string use_path = GetFilePath(use_loc_);
  // If the symbol has a special mapping, use it, otherwise map its file.
  public_headers_ = picker.GetCandidateHeadersForSymbolUsedFrom(
      symbol_name_, use_path);
  if (public_headers_.empty())
    public_headers_ = picker.GetCandidateHeadersForFilepathIncludedFrom(
        decl_filepath_, use_path);
  if (public_headers_.empty())
    public_headers_.push_back(ConvertToQuotedInclude(decl_filepath_));
}

const vector<string>& OneUse::public_headers() {
  if (public_headers_.empty()) {
    SetPublicHeaders();
    CHECK_(!public_headers_.empty() && "Should always have at least one hdr");
  }
  return public_headers_;
}

bool OneUse::PublicHeadersContain(const string& elt) {
  // TODO(csilvers): get rid of this method.
  return ContainsValue(public_headers(), elt);
}

bool OneUse::NeedsSuggestedHeader() const {
  return (!ignore_use() && is_full_use() && suggested_header_.empty());;
}

namespace internal {

// At verbose level 7 and above, returns a printable version of
// the pointer, suitable for being emitted after AnnotatedName.
// At lower verbose levels, returns the empty string.
string PrintablePtr(const void* ptr) {
  if (ShouldPrint(7)) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%p ", ptr);
    return buffer;
  }
  return "";
}

// Helpers for printing a forward declaration of a record type or
// record type template that can be put in source code.  The hierarchy
// of the Decl classes used in these helpers looks like:
//
//   NamedDecl
//   |-- NamespaceDecl
//   |-- TemplateDecl
//   `-- TypeDecl
//       `-- TagDecl           (class, struct, union, enum)
//           `-- RecordDecl    (class, struct, union)

// Given a NamedDecl that presents a (possibly template) record
// (i.e. class, struct, or union) type declaration, and the print-out
// of its (possible) template parameters and kind (e.g. "template
// <typename T> struct"), returns its forward declaration line.
string PrintForwardDeclare(const NamedDecl* decl,
                           const string& tpl_params_and_kind,
                           bool cxx17ns) {
  // We need to short-circuit the logic for testing.
  if (const FakeNamedDecl* fake = FakeNamedDeclIfItIsOne(decl)) {
    return tpl_params_and_kind + " " + fake->qual_name() + ";";
  }

  CHECK_((isa<RecordDecl>(decl) || isa<TemplateDecl>(decl)) &&
         "IWYU only allows forward declaring (possibly template) record types");

  std::string fwd_decl = std::string(decl->getName()) + ";";
  bool seen_namespace = false;
  // Anonymous namespaces are not using the more concise syntax.
  bool concat_namespaces = cxx17ns && !decl->isInAnonymousNamespace();
  for (const DeclContext* ctx = decl->getDeclContext();
       ctx && isa<NamedDecl>(ctx); ctx = ctx->getParent()) {
    if (const RecordDecl* rec = DynCastFrom(ctx)) {
      fwd_decl = std::string(rec->getName()) + "::" + fwd_decl;
    } else if (const NamespaceDecl* ns = DynCastFrom(ctx)) {
      bool first = !seen_namespace;
      if (!seen_namespace) {
        seen_namespace = true;
        fwd_decl = tpl_params_and_kind + " " + fwd_decl;
      }

      if (concat_namespaces) {
        std::string ns_name = std::string(ns->getName());
        std::string prefix = ns_name;
        std::string suffix;
        if (first) {
          first = false;
          prefix = prefix + " { ";
        }
        if (ctx->getParent() && isa<NamedDecl>(ctx->getParent())) {
          prefix = "::" + prefix;
        } else {
          prefix = "namespace " + prefix;
          suffix = " }";
        }
        fwd_decl = prefix + fwd_decl + suffix;
      } else {
        std::string ns_name = ns->isAnonymousNamespace() ?
            std::string() : (std::string(ns->getName()) + " ");
        fwd_decl = "namespace " + ns_name + "{ " + fwd_decl + " }";
      }
    } else if (const FunctionDecl* fn = DynCastFrom(ctx)) {
      // A local class (class defined inside a function).
      fwd_decl = std::string(fn->getName()) + "::" + fwd_decl;
    } else {
      CHECK_UNREACHABLE_("Unexpected decoration for type");
    }
  }

  if (!seen_namespace) {
    fwd_decl = tpl_params_and_kind + " " + fwd_decl;
  }
  return fwd_decl;
}

// Given a RecordDecl, return the line that could be put in source
// code to forward-declare the record type, e.g. "namespace ns { class Foo; }".
string MungedForwardDeclareLineForNontemplates(const RecordDecl* decl) {
  return PrintForwardDeclare(decl, GetKindName(decl), GlobalFlags().cxx17ns);
}

// Given a TemplateDecl representing a class|struct|union template
// declaration, return the line that could be put in source code to
// forward-declare the template, e.g.
//     "namespace ns { template <typename T> class Foo; }".
string MungedForwardDeclareLineForTemplates(const TemplateDecl* decl) {
  // DeclPrinter prints the class name just as we like it (with
  // default args and everything) -- with logic that doesn't exist
  // elsewhere in clang that I can see.  Unfortunately, it also prints
  // the full class body.  So, as a hack, we use PrintableDecl to get
  // the full declaration, and then hack off everything after the
  // template name.  We also have to replace the name with the fully
  // qualified name.  TODO(csilvers): prepend namespaces instead.
  std::string line;       // llvm wants regular string, not our versa-string
  raw_string_ostream ostream(line);
  decl->print(ostream);   // calls DeclPrinter
  line = ostream.str();
  // Get rid of the superclasses, if any (this will nix the body too).
  line = Split(line, " :", 2)[0];
  // Get rid of the template body, if any (true if no superclasses).
  line = Split(line, " {", 2)[0];

  // Remove "final" specifier which isn't needed for forward
  // declarations.
  const char kFinalSpecifier[] = " final ";
  string::size_type final_pos = line.find(kFinalSpecifier);
  if (final_pos != string::npos) {
    line.replace(final_pos, sizeof(kFinalSpecifier), " ");
  }

  // The template name is now the last word on the line.  Replace it
  // by its fully-qualified form.
  const string::size_type name = line.rfind(' ');
  CHECK_(name != string::npos && "Unexpected printable template-type");

  return PrintForwardDeclare(decl, line.substr(0, name), GlobalFlags().cxx17ns);
}

string MungedForwardDeclareLine(const NamedDecl* decl) {
  if (const RecordDecl* rec_decl = DynCastFrom(decl))
    return MungedForwardDeclareLineForNontemplates(rec_decl);
  else if (const TemplateDecl* template_decl = DynCastFrom(decl))
    return MungedForwardDeclareLineForTemplates(template_decl);
  CHECK_UNREACHABLE_("Unexpected decl type for MungedForwardDeclareLine");
}

}  // namespace internal

OneIncludeOrForwardDeclareLine::OneIncludeOrForwardDeclareLine(
    const NamedDecl* fwd_decl)
    : line_(internal::MungedForwardDeclareLine(fwd_decl)),
      start_linenum_(-1),   // set 'for real' below
      end_linenum_(-1),     // set 'for real' below
      is_desired_(false),
      is_present_(false),
      included_file_(nullptr),
      fwd_decl_(fwd_decl) {
  const SourceRange decl_lines = GetSourceRangeOfClassDecl(fwd_decl);
  // We always want to use the instantiation line numbers: for code like
  //     FORWARD_DECLARE_CLASS(MyClass);
  // we care about where this macro is called, not where it's defined.
  start_linenum_ = GetLineNumber(GetInstantiationLoc(decl_lines.getBegin()));
  end_linenum_ = GetLineNumber(GetInstantiationLoc(decl_lines.getEnd()));
}

OneIncludeOrForwardDeclareLine::OneIncludeOrForwardDeclareLine(
    const FileEntry* included_file, const string& quoted_include, int linenum)
    : line_("#include " + quoted_include),
      start_linenum_(linenum),
      end_linenum_(linenum),
      is_desired_(false),
      is_present_(false),
      quoted_include_(quoted_include),
      included_file_(included_file),
      fwd_decl_(nullptr) {
}

bool OneIncludeOrForwardDeclareLine::HasSymbolUse(const string& symbol_name)
    const {
  return ContainsKey(symbol_counts_, symbol_name);
}

void OneIncludeOrForwardDeclareLine::AddSymbolUse(const string& symbol_name) {
  ++symbol_counts_[symbol_name];
}

bool OneIncludeOrForwardDeclareLine::IsIncludeLine() const {
  // Since we construct line_, we know it's in canonical form, and
  // can't look like '  #   include   <foo.h>' or some such.
  return StartsWith(line_, "#include");
}

string OneIncludeOrForwardDeclareLine::LineNumberString() const {
  char buf[64];   // big enough for any two numbers
  snprintf(buf, sizeof(buf), "%d-%d", start_linenum_, end_linenum_);
  return buf;
}

IwyuFileInfo::IwyuFileInfo(const clang::FileEntry* this_file,
                           const IwyuPreprocessorInfo* preprocessor_info,
                           const string& quoted_include_name)
  : file_(this_file),
    preprocessor_info_(preprocessor_info),
    quoted_file_(quoted_include_name),
    is_prefix_header_(false),
    is_pch_in_code_(false),
    desired_includes_have_been_calculated_(false)
{}

void IwyuFileInfo::AddAssociatedHeader(const IwyuFileInfo* other) {
  VERRS(6) << "Adding " << GetFilePath(other->file_)
           << " as associated header for " << GetFilePath(file_) << "\n";
  associated_headers_.insert(other);
}

void IwyuFileInfo::AddInclude(const clang::FileEntry* includee,
                              const string& quoted_includee, int linenumber) {
  OneIncludeOrForwardDeclareLine new_include(includee, quoted_includee,
                                             linenumber);
  new_include.set_present();

  // It's possible for the same #include to be seen multiple times
  // (for instance, if we include a .h file twice, and that .h file
  // does not have a header guard).  Ignore all but the first.
  // TODO(csilvers): could rewrite this so it's constant-time.
  for (const OneIncludeOrForwardDeclareLine& line : lines_) {
    if (line.LineNumbersMatch(new_include)) {
      VERRS(6) << "Ignoring repeated include: "
               << GetFilePath(file_) << ":" << linenumber
               << " -> " << GetFilePath(includee) << "\n";
      return;
    }
  }

  lines_.push_back(new_include);
  // Store in a few other ways as well.
  direct_includes_as_fileentries_.insert(includee);
  direct_includes_.insert(quoted_includee);
  VERRS(6) << "Found include: "
           << GetFilePath(file_) << ":" << linenumber
           << " -> " << GetFilePath(includee) << "\n";
}

void IwyuFileInfo::AddForwardDeclare(const clang::NamedDecl* fwd_decl,
                                     bool definitely_keep_fwd_decl) {
  CHECK_(fwd_decl && "forward_declare_decl unexpectedly nullptr");
  CHECK_((isa<ClassTemplateDecl>(fwd_decl) || isa<RecordDecl>(fwd_decl))
         && "Can only forward declare classes and class templates");
  lines_.push_back(OneIncludeOrForwardDeclareLine(fwd_decl));
  lines_.back().set_present();
  if (definitely_keep_fwd_decl)
    lines_.back().set_desired();
  direct_forward_declares_.insert(fwd_decl);   // store in another way as well
  VERRS(6) << "Found forward-declare: "
           << GetFilePath(file_) << ":" << lines_.back().LineNumberString()
           << ": " << internal::PrintablePtr(fwd_decl)
           << internal::GetQualifiedNameAsString(fwd_decl) << "\n";
}

void IwyuFileInfo::AddUsingDecl(const UsingDecl* using_decl) {
  CHECK_(using_decl && "using_decl unexpectedly nullptr");
  using_decl_referenced_.insert(std::make_pair(using_decl, false));
  const SourceRange decl_lines = using_decl->getSourceRange();
  int start_linenum = GetLineNumber(GetInstantiationLoc(decl_lines.getBegin()));
  int end_linenum = GetLineNumber(GetInstantiationLoc(decl_lines.getEnd()));
  VERRS(6) << "Found using-decl: "
           << GetFilePath(file_) << ":" 
           << to_string(start_linenum) << "-" << to_string(end_linenum) << ": " 
           << internal::PrintablePtr(using_decl)
           << internal::GetQualifiedNameAsString(using_decl) << "\n";
}

static void LogSymbolUse(const string& prefix, const OneUse& use) {
  string decl_loc;
  string printable_ptr;
  if (use.decl()) {
    decl_loc = PrintableLoc(GetLocation(use.decl()));
    printable_ptr = internal::PrintablePtr(use.decl());
  } else {
    decl_loc = use.decl_filepath();
  }
  VERRS(6) << prefix << " " << printable_ptr << use.symbol_name()
           << " (from " << decl_loc << ")"
           << " at " << use.PrintableUseLoc() << "\n";
}

void IwyuFileInfo::ReportFullSymbolUse(SourceLocation use_loc,
                                       const NamedDecl* decl,
                                       UseFlags flags,
                                       const char* comment) {
  if (decl) {
    const NamedDecl* report_decl;
    SourceLocation report_decl_loc;

    if ((flags & (UF_FunctionDfn | UF_ExplicitInstantiation)) == 0) {
      // Since we need the full symbol, we need the decl's definition-site too.
      // Also, by default we canonicalize the location, using GetLocation.
      report_decl = GetDefinitionAsWritten(decl);
      report_decl_loc = GetLocation(report_decl);
    } else {
      // However, if we're defining the function or we are targeting an explicit
      // instantiation, we want to use it as-is and not try to canonicalize at all.
      report_decl = decl;
      report_decl_loc = decl->getLocation();
    }

    symbol_uses_.push_back(OneUse(report_decl, use_loc, report_decl_loc,
                                  OneUse::kFullUse, flags, comment));
    LogSymbolUse("Marked full-info use of decl", symbol_uses_.back());
  }
}

void IwyuFileInfo::ReportFullSymbolUse(SourceLocation use_loc,
                                       const string& dfn_filepath,
                                       const string& symbol) {
  symbol_uses_.push_back(OneUse(symbol, nullptr, dfn_filepath, use_loc));
  LogSymbolUse("Marked full-info use of symbol", symbol_uses_.back());
}

void IwyuFileInfo::ReportMacroUse(clang::SourceLocation use_loc,
                                  clang::SourceLocation dfn_loc,
                                  const string& symbol) {
  symbol_uses_.push_back(OneUse(symbol, GetFileEntry(dfn_loc),
                                GetFilePath(dfn_loc), use_loc));
  LogSymbolUse("Marked full-info use of macro", symbol_uses_.back());
}

void IwyuFileInfo::ReportDefinedMacroUse(const clang::FileEntry* used_in) {
  macro_users_.insert(used_in);
}

void IwyuFileInfo::ReportIncludeFileUse(const clang::FileEntry* included_file,
                                        const string& quoted_include) {
  symbol_uses_.push_back(OneUse("", included_file, quoted_include,
                                SourceLocation()));
  LogSymbolUse("Marked use of include-file", symbol_uses_.back());
}

void IwyuFileInfo::ReportKnownDesiredFile(const FileEntry* included_file) {
  kept_includes_.insert(included_file);
}

void IwyuFileInfo::ReportForwardDeclareUse(SourceLocation use_loc,
                                           const NamedDecl* decl,
                                           UseFlags flags,
                                           const char* comment) {
  if (!decl)
    return;
  // Sometimes, a bug in clang (http://llvm.org/bugs/show_bug.cgi?id=8669)
  // combines friend decls with true forward-declare decls.  If that
  // happened here, replace the friend with a real fwd decl.
  decl = GetNonfriendClassRedecl(decl);
  symbol_uses_.push_back(OneUse(decl, use_loc, GetLocation(decl),
                                OneUse::kForwardDeclareUse, flags, comment));
  LogSymbolUse("Marked fwd-decl use of decl", symbol_uses_.back());
}

void IwyuFileInfo::ReportUsingDeclUse(SourceLocation use_loc,
                                      const UsingDecl* using_decl,
                                      UseFlags flags,
                                      const char* comment) {  
  // If accessing a symbol through a using decl in the same file that contains
  // the using decl, we must mark the using decl as referenced. At the end of
  // traversing the AST, we check to see if a using decl is unreferenced and
  // add a full use of one of its shadow decls so that the source file
  // continues to compile.
  auto using_decl_status = using_decl_referenced_.find(using_decl);

  if (using_decl_status != using_decl_referenced_.end()) {
    using_decl_status->second = true;
  }

  // When a symbol is accessed through a using decl, we must report
  // that as a full use of the using decl because whatever file that
  // using decl is in is now required.
  ReportFullSymbolUse(use_loc, using_decl, flags, comment);
}

// Given a collection of symbol-uses for symbols defined in various
// files, figures out the minimal set of #includes needed to get those
// definitions.  Typically this is a trivial task: if we need the full
// information from a decl, we just have to #include the header file
// with the decl's definition.  But if that header file is a private
// decl -- e.g. <bits/stl_vector.h> -- we need to map that to a public
// decl first.  And if more than one public decl fits the bill, we
// want to pick the one that minimizes the number of new #includes
// added.  Stores its results by updating the input vector of
// OneUse's.  For convenience, returns the set of "desired" includes:
// all includes that were added to suggested_header.

static void LogIncludeMapping(const string& reason, const OneUse& use) {
  VERRS(6) << "Mapped " << use.decl_filepath() << " to "
           << use.suggested_header() << " for " << use.symbol_name()
           << " (" << reason << ")\n";
}

namespace internal {

bool DeclCanBeForwardDeclared(const Decl* decl, string* reason) {
  // Nothing inside an inline namespace can be forward-declared.
  if (IsInInlineNamespace(decl)) {
    *reason = "in inline namespace";
    return false;
  }

  if (isa<ClassTemplateDecl>(decl)) {
    // Class templates can always be forward-declared.
  } else if (const auto* record = dyn_cast<RecordDecl>(decl)) {
    // Record decls can be forward-declared unless they denote a lambda
    // expression; these have no type name to forward-declare.
    if (record->isLambda()) {
      *reason = "is a lambda";
      return false;
    }
  } else {
    // Other decl types are not forward-declarable.
    *reason = "not a record or class template";
    return false;
  }

  return true;
}

bool DeclCanBeForwardDeclared(const Decl* decl) {
  string reason;
  return DeclCanBeForwardDeclared(decl, &reason);
}

// Helper to tell whether a forward-declare use is 'preceded' by a
// declaration inside the same file.  'Preceded' is in quotes, because
// it's actually ok if the declaration follows the use, inside a
// class.  (You can write a method using a Foo* before defining the
// nested class Foo later in the class.)
bool DeclIsVisibleToUseInSameFile(const Decl* decl, const OneUse& use) {
  if (GetFileEntry(decl) != GetFileEntry(use.use_loc()))
    return false;

  // If the decl comes before the use, it's visible to it.  (The
  // decl can also be at the same location as the use, e.g. for
  //   struct Foo { int x, y; } myvar
  // )  It can even be visible if the decl comes after, if the decl
  // is inside the class definition and the use is in the body of a
  // method.
  return (IsBeforeInSameFile(decl, use.use_loc()) ||
          GetLocation(decl) == use.use_loc() ||
          (DeclsAreInSameClass(decl, use.decl()) && !decl->isOutOfLine() &&
           (use.flags() & UF_InCxxMethodBody)));
}

// This makes a best-effort attempt to find the smallest set of
// #include files that satisfy all uses.  A more accurate name
// might be "calculate minimal-ish includes". :-)  It populates
// each OneUse in uses with the best #include for that use.
// direct_includes: this file's direct includes only.
// associated_direct_includes: direct includes for 'associated'
// files.  For everything but foo.cc, this is empty; for foo.cc it's
// foo.h's includes and foo-inl.h's includes.
set<string> CalculateMinimalIncludes(
    const string& use_quoted_include,
    const set<string>& direct_includes,
    const set<string>& associated_direct_includes,
    vector<OneUse>* uses) {
  set<string> desired_headers;

  // TODO(csilvers): if a use's decl supports equivalent redecls
  // (such as a FunctionDecl or TypedefDecl), pick the redecl
  // that yields the "best" #include.

  // Step (1) The easy case: decls that map to just one file.  This
  // captures both decls that aren't in private header files, and
  // those in private header files that only map to one public file.
  // For every other decl, we store the (decl, public-headers) pair.
  for (OneUse& use : *uses) {
    // We don't need to add any #includes for non-full-use.
    if (use.ignore_use() || !use.is_full_use())
      continue;
    // Special case #1: Some uses come with a suggested header already picked.
    if (use.has_suggested_header()) {
      desired_headers.insert(use.suggested_header());
      continue;
    }
    // Special case #2: if the dfn-file maps to the use-file, then
    // this is a file that the use-file is re-exporting symbols for,
    // and we should keep the #include as-is.
    const string use_file = ConvertToQuotedInclude(GetFilePath(use.use_loc()));
    const string decl_file = ConvertToQuotedInclude(use.decl_filepath());
    if (use.PublicHeadersContain(use_file) &&
        ContainsKey(direct_includes, decl_file)) {
      use.set_suggested_header(decl_file);
      desired_headers.insert(use.suggested_header());
      LogIncludeMapping("private header", use);
    } else if (use.public_headers().size() == 1) {
      use.set_suggested_header(use.public_headers()[0]);
      desired_headers.insert(use.suggested_header());
      LogIncludeMapping("only candidate", use);
    }
  }

  // Steps (2): Go through the needed private-includes that map to
  // more than one public #include.  Use the following priority order:
  // - Ourselves.
  // - An include in associated_direct_includes (those are includes
  //   that are not going away, since we can't change associated
  //   files).
  // - Includes in direct_includes that are also already in
  //   desired_headers.
  // - Includes in desired_headers.
  // - Includes in direct_includes.
  // Picking in this order minimizes the number of #includes we add,
  // while allowing us to remove #includes if need be.
  for (OneUse& use : *uses) {
    if (!use.NeedsSuggestedHeader())
      continue;
    const vector<string>& public_headers = use.public_headers();
    for (const string& choice : public_headers) {
      if (use.has_suggested_header())
        break;
      if (choice == use_quoted_include) {
        use.set_suggested_header(choice);
        LogIncludeMapping("in self", use);
      }
    }
    // TODO(csilvers): write ElementInBoth() in iwyu_stl_util.h
    for (const string& choice : public_headers) {
      if (use.has_suggested_header())
        break;
      if (ContainsKey(associated_direct_includes, choice)) {
        use.set_suggested_header(choice);
        desired_headers.insert(use.suggested_header());
        LogIncludeMapping("in associated header", use);
      }
    }
    for (const string& choice : public_headers) {
      if (use.has_suggested_header())
        break;
      if (ContainsKey(direct_includes, choice) &&
          ContainsKey(desired_headers, choice)) {
        use.set_suggested_header(choice);
        desired_headers.insert(use.suggested_header());
        LogIncludeMapping("#include already present and needed", use);
      }
    }
    for (const string& choice : public_headers) {
      if (use.has_suggested_header())
        break;
      if (ContainsKey(desired_headers, choice)) {
        use.set_suggested_header(choice);
        desired_headers.insert(use.suggested_header());
        LogIncludeMapping("#include already needed", use);
      }
    }
    for (const string& choice : public_headers) {
      if (use.has_suggested_header())
        break;
      if (ContainsKey(direct_includes, choice)) {
        use.set_suggested_header(choice);
        desired_headers.insert(use.suggested_header());
        LogIncludeMapping("#include already present", use);
      }
    }
  }

  // Step (3): Now we have a set-cover problem: we need to end up with
  // a set of headers, called cover, so that for every i:
  //    intersection(cover, public_headers[i]) != empty_set
  // We do this greedily: we find the header that's listed the most
  // often.  Among those, we prefer the one that's listed first in
  // public_headers[i] the most often (each list is in approximate
  // best-fit order).  Among those, we choose arbitrarily.  We repeat
  // until we cover all sets.
  set<OneUse*> unmapped_uses;
  for (OneUse& use : *uses) {
    if (use.NeedsSuggestedHeader())
      unmapped_uses.insert(&use);
  }
  while (!unmapped_uses.empty()) {
    map<string, pair<int,int>> header_counts;   // total appearances, 1st's
    for (OneUse* use : unmapped_uses) {
      CHECK_(!use->has_suggested_header());
      const vector<string>& public_headers = use->public_headers();
      for (const string& choice : public_headers) {
        if (use->has_suggested_header())
          break;
        ++header_counts[choice].first;  // increment total count
        if (choice == use->public_headers()[0])
          ++header_counts[choice].second;  // increment first-in-list count
      }
    }
    pair<string, pair<int, int>> best = *header_counts.begin();
    for (const auto& header_count : header_counts) {
      // Use pair<>'s operator> to order for us.
      if (header_count.second > best.second)
        best = header_count;
    }
    const string hdr = best.first;
    desired_headers.insert(hdr);

    // Now go through and assign to symbols satisfied by this header.
    for (set<OneUse*>::iterator it = unmapped_uses.begin();
         it != unmapped_uses.end(); ) {
      if ((*it)->NeedsSuggestedHeader() && (*it)->PublicHeadersContain(hdr)) {
        (*it)->set_suggested_header(hdr);
        LogIncludeMapping("set cover", *(*it));
        // set<> has nice property that erasing doesn't invalidate iterators.
        unmapped_uses.erase(it++);   // because we just mapped it!
      } else {
        ++it;
      }
    }
  }
  return desired_headers;
}

// Calculating iwyu violations is a multi-step process.  The basic
// idea is we trim the existing uses to ones that might plausibly be
// iwyu violations, for both forward-declares (A) and full uses (B).
// Then we calculate the desired (end-result) set of #includes (C).
// After that we can do suggested trimming, with knowledge of all
// #includes, to reduce to full-use (D) and forward-declare uses (E)
// that are actually iwyu violations.
//
// Trimming forward-declare uses (1st pass):
// A1) If not a class or a templated class, recategorize as a full use.
// A2) If a templated class with default template params, recategorize
//     as a full use (forward-declaring in that case is too error-prone).
// A3) If a symbol in std, __gnu_cxx, or another system namespace,
//     recategorize as a full use.  This is entirely a policy
//     decision: we've decided never to forward-declare anything in
//     a system namespace, because it's best not to expose the internals
//     of system headers in user code, if possible.
// A4) If the file containing the use has a pragma inhibiting the forward
//     declaration of the symbol, change the use to a full info use in order
//     to make sure that the compiler can see some declaration of the symbol.
// A5) If a nested class, discard this use (the parent class declaration
//     is sufficient).
// A6) If any of the redeclarations of this declaration is in the same
//     file as the use (and before it), and is actually a definition,
//     discard the forward-declare use.
// A7) If --no_fwd_decls has been passed, recategorize as a full use.

// Trimming symbol uses (1st pass):
// B1) If the definition of a full use comes after the use, change the
//     full use to a forward-declare use that points to a fwd-decl
//     that comes before the use.  (This is for cases like typedefs
//     where iwyu demands a full use but the language allows a
//     forward-declare.)
// B2) Discard symbol uses of a symbol defined in the same file it's used.
//     If the symbol is an enum, typedef, function, or var -- every decl
//     that is re-declarable except for RecordDecl -- discard if *any*
//     declaration is in the same file as the use.
// B3) Discard symbol uses for builtin symbols ('__builtin_memcmp') and
//     for operator new and operator delete (excluding placement new),
//     which are effectively built-in even though they're in <new>.
// B4) Discard symbol uses for member functions that live in the same
//     file as the class they're part of (the parent check suffices).
// B5) Sanity check: Discard 'backwards' #includes.  These are
//     #includes where we say a.h should #include b.h, but b.h is
//     already #including a.h.  This happens when iwyu attributes a
//     use to the wrong file.
// B6) In --transitive_includes_only mode, discard 'new' #includes.
//     These are #includes where we say a.h should #include b.h, but
//     a.h does not see b.h in its transitive #includes.  (Note: This
//     happens before include-picker mapping, so it's still possible to
//     see 'new' includes via a manual mapping.)
// B1') Discard macro uses in the same file as the definition (B2 redux).
// B2') Discard macro uses that form a 'backwards' #include (B5 redux).
// B3') Discard macro uses from a 'new' #include (B6 redux).

// Determining 'desired' #includes:
// C1) Get a list of 'effective' direct includes.  For most files, it's
//     the same as the actual direct includes, but for the main .cc
//     file it also gets 'free' includes from its associated .h files.
// C2) For each symbol-use, calculate the set of public header files that
//     'provide' that symbol (e.g. <stddef.h> and <stdlib.h> for NULL).
// C3) Find the minimal 'set cover' over these sets: find a "add-minimal"
//     collection of files that has overlap with every set from (1).
//     "Add-minimal" means that the collection should have as few
//     files in it as possible *that we are not already #including*.
// C4) Sanity check: remove any .cc files from desired-includes unless
//     they're already in actual-includes.
//
// Calculate IWYU violations for forward-declares:
// D1) If the definition of the forward-declaration lives in a desired
//     include, or any redecl lives in the current file (and earlier
//     in the file), reassign decl_ to point to that redecl; if the
//     decl is not in the current file, mark the filename the decl
//     comes from.
// D2) If the definition is not in current includes, and no redecl is
//     in the current file (and earlier in the file), mark as an iwyu
//     violation.
//
// Calculate IWYU violations for full uses:
// E1) Sanity check: ignore the use if it would require adding an
//     #include of a .cc file.
// E2) Ignore use when the suggested header *is* the current file
//     (this can happen due to mappings).
// E3) If the desired include-file for this symbols is not in the
//     current includes, mark as an iwyu violation.

void ProcessForwardDeclare(OneUse* use,
                           const IwyuPreprocessorInfo* preprocessor_info) {
  CHECK_(use->decl() && "Must call ProcessForwardDeclare on a decl");
  CHECK_(!use->is_full_use() && "Must call ProcessForwardDeclare on fwd-decl");
  if (use->ignore_use())   // we're already ignoring it
    return;

  // (A1) If not suitable for forward-declaration, recategorize as a full use.
  string reason;
  if (!DeclCanBeForwardDeclared(use->decl(), &reason)) {
    VERRS(6) << "Moving " << use->symbol_name()
             << " from fwd-decl use to full use: " << reason << " ("
             << use->PrintableUseLoc() << ")\n";
    use->set_full_use();
    return;
  }
  // This is useful for the subsequent tests -- let's normalize some types.
  const RecordDecl* record_decl = DynCastFrom(use->decl());
  const ClassTemplateDecl* tpl_decl = DynCastFrom(use->decl());
  const ClassTemplateSpecializationDecl* spec_decl = DynCastFrom(use->decl());
  if (spec_decl)
    tpl_decl = spec_decl->getSpecializedTemplate();
  if (tpl_decl)
    record_decl = tpl_decl->getTemplatedDecl();

  // (A2) If it has default template parameters, recategorize as a full use.
  // Suppress this if there's no definition for this class (so can't full-use).
  if (tpl_decl && HasDefaultTemplateParameters(tpl_decl) &&
      GetDefinitionForClass(tpl_decl) != nullptr) {
    VERRS(6) << "Moving " << use->symbol_name()
             << " from fwd-decl use to full use: has default template param"
             << " (" << use->PrintableUseLoc() << ")\n";
    use->set_full_use();
    // No return here: (A4) or (A5) may cause us to ignore this decl entirely.
  }

  // (A3) If it is in namespace std or a system ns, recategorize as a full use.
  // We can add new system namespaces here as needed.
  // TODO(csilvers): if someone has specialized a class in std, the
  // specialization should be treated as in user-space and
  // forward-declarable.  Check for that case.
  if (StartsWith(use->symbol_name(), "std::") ||
      StartsWith(use->symbol_name(), "__gnu_cxx::")) {
    VERRS(6) << "Moving " << use->symbol_name()
             << " from fwd-decl use to full use: in a system namespace "
             << " (" << use->PrintableUseLoc() << ")\n";
    use->set_full_use();
    // No return here: (A4) or (A5) may cause us to ignore this decl entirely.
  }

  // (A4) If the file containing the use has a pragma inhibiting the forward
  // declaration of the symbol, change the use to a full info use in order
  // to make sure that the compiler can see some declaration of the symbol.
  if (!use->is_full_use()) {
    if (preprocessor_info->ForwardDeclareIsInhibited(
            GetFileEntry(use->use_loc()), use->symbol_name())) {
      VERRS(6) << "Changing fwd-decl use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc()
               << ") to a full-info use: no_forward_declare pragma\n";
      use->set_full_use();
    }
  }

  // (A5) If using a nested class, discard this use.
  if (IsNestedClass(record_decl)) {
    // iwyu will require the full type of the parent class when it
    // recurses on the qualifier (any use of Foo::Bar requires the
    // full type of Foo).  So if we're forward-declared inside Foo,
    // the user will get that forward-declaration for free when
    // it gets the full definition of Foo.  The one exception is
    // when the use is itself inside the class, in which case it
    // sometimes needs the forward-declaration: for instance
    //    class Foo { class Nested; Nested* Fn(); class Nested { ... } };
    // This exception applies only when the use is in the same class
    // as the decl; we'll be conservative and apply it whenever
    // they're in the same file.
    if (GetFileEntry(use->use_loc()) != GetFileEntry(use->decl())) {
      VERRS(6) << "Ignoring fwd-decl use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc() << "): nested class\n";
      use->set_ignore_use();
      return;
    }
  }

  // (A6) If a definition exists earlier in this file, discard this use.
  // Note: for the 'earlier' checks, what matters is the *instantiation*
  // location.
  const set<const NamedDecl*> redecls = GetClassRedecls(record_decl);
  for (const NamedDecl* redecl : redecls) {
    CHECK_(isa<RecordDecl>(redecl) &&
           "GetClassRedecls has redecls of wrong type");
    const SourceLocation defined_loc = GetLocation(redecl);
    if (cast<RecordDecl>(redecl)->isCompleteDefinition() &&
        DeclIsVisibleToUseInSameFile(redecl, *use)) {
      VERRS(6) << "Ignoring fwd-decl use of " << use->symbol_name() << " ("
               << use->PrintableUseLoc()
               << "): dfn is present: " << PrintableLoc(defined_loc) << "\n";
      use->set_ignore_use();
      return;
    }
  }

  // (A7) If --no_fwd_decls has been passed, and a decl can be found in one of
  // the headers, suggest that header, and recategorize as a full use. If we can
  // only find a decl in this file, it must be a self-sufficent decl being used,
  // so we can just let IWYU do its work, and there is no need to recategorize.
  if (!use->ignore_use() && GlobalFlags().no_fwd_decls) {
    bool promote_to_full_use = true;
    for (const Decl* decl = use->decl(); decl != nullptr;
         decl = decl->getPreviousDecl()) {
      if (IsBeforeInSameFile(decl->getLocation(), use->use_loc())) {
        promote_to_full_use = false;
      } else if (IsBeforeInTranslationUnit(decl->getLocation(),
                                           use->use_loc())) {
        // TODO: Choose a redecl that is already provided by a desired include, so we
        // don't keep another include that is not necessary.
        use->reset_decl(cast<NamedDecl>(decl));
        promote_to_full_use = true;
        break;
      }
    }

    if (promote_to_full_use) {
      use->set_full_use();
    }
  }
}

// Returns true if the given symbol has a mapping defined to a file.
static bool HasMapping(const string& symbol) {
  return !GlobalIncludePicker().GetCandidateHeadersForSymbol(symbol).empty();
}

void ProcessFullUse(OneUse* use,
                    const IwyuPreprocessorInfo* preprocessor_info) {
  CHECK_(use->decl() && "Must call ProcessFullUse on a decl");
  CHECK_(use->is_full_use() && "Must not call ProcessFullUse on fwd-decl");
  if (use->ignore_use())   // we're already ignoring it
    return;

  // We normally ignore uses for builtins, but when there is a mapping defined
  // for the symbol, we should respect that.  So, we need to determine whether
  // the symbol has any mappings.
  bool is_builtin_function = IsBuiltinFunction(use->decl(), use->symbol_name());

  bool is_builtin_function_with_mappings =
      is_builtin_function && HasMapping(use->symbol_name());

  // (B1) If the definition is after the use, re-point to a prior decl.
  // If iwyu followed the language precisely, this wouldn't be
  // necessary: code wouldn't compile if a full-use didn't have the
  // definition handy yet.  But in fact, iwyu sometimes requires a full
  // type when the language doesn't, notably with typedefs.  For code
  // like 'struct f; typedef f g; struct f {};', iwyu will say the
  // typedef requires a definition of f, and as a result will say the
  // forward-decl is unnecessary (who cares about forward-decls when
  // we need a definition?), when in fact it's crucial.
  // For now, we assume a 'later' usage must be in the same file.
  if (GetFileEntry(use->use_loc()) == GetFileEntry(use->decl()) &&
      !DeclIsVisibleToUseInSameFile(use->decl(), *use) &&
      DeclCanBeForwardDeclared(use->decl())) {
    if (preprocessor_info->ForwardDeclareIsInhibited(
            GetFileEntry(use->use_loc()), use->symbol_name())) {
      // There is no include we could recommend for any full use, so just
      // ignore the use.
      VERRS(6) << "Ignoring use of " << use->symbol_name()
               << ": definition found later in file"
               << " and no_forward_declare pragma present("
               << use->PrintableUseLoc() << ")\n";
      use->set_ignore_use();
      return;
    }
    // Just change us to a forward-declare use.  Later, we'll decide
    // which forward-declare is the best one to keep.
    VERRS(6) << "Moving " << use->symbol_name()
             << " from full use to fwd-decl: definition found later in file"
             << " (" << use->PrintableUseLoc() << ")\n";
    use->set_forward_declare_use();
    return;
  }

  // (B2) Discard symbol uses of a symbol defined in the same file it's used.
  // If the symbol can be declared in multiple places, we count it if
  // *any* declaration is in the same file, unless the symbol is a
  // class.  (Every other kind of redeclarable symbol, such as
  // functions, have the property that a decl is the same as a
  // definition from iwyu's point of view.)  We don't bother with
  // RedeclarableTemplate<> types (FunctionTemplateDecl), since for
  // those types, iwyu *does* care about the definition vs declaration.
  // All this is moot when FunctionDecls are being defined, all their redecls
  // are separately registered as uses so that a definition anchors all its
  // declarations.
  if (!(use->flags() & UF_FunctionDfn) && !is_builtin_function_with_mappings) {
    set<const NamedDecl*> all_redecls;
    if (isa<RecordDecl>(use->decl()) || isa<ClassTemplateDecl>(use->decl()))
      all_redecls.insert(use->decl());  // for classes, just consider the dfn
    else
      all_redecls = GetNonclassRedecls(use->decl());
    for (const NamedDecl* redecl : all_redecls) {
      if (DeclIsVisibleToUseInSameFile(redecl, *use)) {
        VERRS(6) << "Ignoring use of " << use->symbol_name() << " ("
                 << use->PrintableUseLoc() << "): definition is present: "
                 << PrintableLoc(GetLocation(use->decl())) << "\n";
        use->set_ignore_use();
        return;
      }
    }
  }

  // (B3) Discard symbol uses for builtin symbols, including new/delete and
  // template builtins.
  if (isa<clang::BuiltinTemplateDecl>(use->decl())) {
    VERRS(6) << "Ignoring use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): built-in template\n";
    use->set_ignore_use();
    return;
  }
  // A compiler builtin without a predefined header file (e.g. __builtin_..)
  if (is_builtin_function && !is_builtin_function_with_mappings) {
    VERRS(6) << "Ignoring use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): built-in function\n";
    use->set_ignore_use();
    return;
  }
  // Special case for operators new/delete: Only treated as built-in if they
  // are the default, non-placement versions. This is modelled in Clang as
  // 'replaceable global allocation functions': the helper method returns true
  // for anything but placement-new. Users of the 'std::nothrow' and
  // 'std::align_val_t' overloads already need to spell these two symbols, so
  // <new> will be required for them without us doing any magic for operator new
  // itself.
  if (const FunctionDecl* fn_decl = DynCastFrom(use->decl())) {
    if (fn_decl->isReplaceableGlobalAllocationFunction(nullptr, nullptr)) {
      VERRS(6) << "Ignoring use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc() << "): built-in new/delete\n";
      use->set_ignore_use();
      return;
    }
  }

  // (B4) Discard symbol uses for member functions in the same file as parent.
  if (const CXXMethodDecl* method_dfn = DynCastFrom(use->decl())) {
    // See if we also recorded a use of the parent.
    const NamedDecl* parent_dfn
        = GetDefinitionAsWritten(method_dfn->getParent());

    const FileEntry* decl_file_entry = GetFileEntry(use->decl_loc());
    const FileEntry* parent_file_entry =
        GetFileEntry(GetInstantiationLoc(GetLocation(parent_dfn)));

    // We want to map the definition-files to their public headers if
    // they're private headers (so bits/stl_vector.h and
    // bits/vector.tcc are counted as the "same" file for this test).
    // To be safe, we only do the mapping if both files have at most
    // one public file they map to (otherwise we don't know which
    // mapping to choose, and it's important we use the one that iwyu
    // will pick later).  TODO(csilvers): figure out that case too.
    const IncludePicker& picker = GlobalIncludePicker();
    const vector<MappedInclude>& method_dfn_files =
        picker.GetCandidateHeadersForFilepath(GetFilePath(decl_file_entry));
    const vector<MappedInclude>& parent_dfn_files =
        picker.GetCandidateHeadersForFilepath(GetFilePath(parent_file_entry));
    bool same_file;
    if (method_dfn_files.size() == 1 && parent_dfn_files.size() == 1) {
      same_file = (method_dfn_files[0].quoted_include ==
          parent_dfn_files[0].quoted_include);
    } else {
      // Fall back on just checking the filenames: can't figure out public.
      same_file = (decl_file_entry == parent_file_entry);
    }
    if (same_file) {
      VERRS(6) << "Ignoring use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc() << "): member of class\n";
      use->set_ignore_use();
      return;
    }
  }

  // (B5) Discard uses of symbols that form a 'backwards' #include.
  // This means that we say a.h is using a symbol in b.h, but b.h
  // already #includes a.h (either directly or indirectly).  Since the
  // include graph should be acyclic, this means that iwyu messed up,
  // either by incorrectly saying it was a.h that is using the symbol
  // (this can happen trying to figure out who 'owns' macro code), or
  // by incorrectly saying it was a use (this can happen with typedefs
  // -- we say the underlying type is 'used' in a different way than
  // the language requires).
  // TODO(csilvers): remove this when we resolve the bugs with macros/typedefs.
  if (preprocessor_info->FileTransitivelyIncludes(
          GetFileEntry(use->decl()), GetFileEntry(use->use_loc())) &&
      !is_builtin_function_with_mappings) {
    VERRS(6) << "Ignoring use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): 'backwards' #include\n";
    use->set_ignore_use();
    return;
  }

  // (B6) In --transitive_includes_only mode, discard 'new' #includes.
  // In practice, if we tell a.h to add an #include that is not in its
  // transitive includes, it's usually (but not always) an iwyu error
  // of some sort.  So we allow a flag to discard such recommendations.
  if (GlobalFlags().transitive_includes_only) {
    if (!preprocessor_info->FileTransitivelyIncludes(
            GetFileEntry(use->use_loc()), GetFileEntry(use->decl()))) {
      VERRS(6) << "Ignoring use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc() << "):"
               << " non-transitive #include\n";
      use->set_ignore_use();
      return;
    }
  }
}

void ProcessSymbolUse(OneUse* use,
                      const IwyuPreprocessorInfo* preprocessor_info) {
  if (use->ignore_use())   // we're already ignoring it
    return;

  const FileEntry* use_file = GetFileEntry(use->use_loc());
  const string quoted_decl_file = ConvertToQuotedInclude(use->decl_filepath());

  // (B1') Like (B2), discard symbol uses in the same file as their definition.
  if (GetFilePath(use->use_loc()) == use->decl_filepath()) {
    VERRS(6) << "Ignoring symbol use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): defined in same file\n";
    use->set_ignore_use();
    return;
  }

  // (B2') Like (B5), discard uses of symbols that create 'backwards' includes.
  // Note we suppress this check if suggested_header_ is already set:
  // that only happens with hard-coded uses, which we shouldn't second guess.
  // TODO(csilvers): like (B5), remove this when we have 'soft' uses.
  if (!use->has_suggested_header() &&
      preprocessor_info->FileTransitivelyIncludes(quoted_decl_file, use_file)) {
    VERRS(6) << "Ignoring use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): 'backwards' #include\n";
    use->set_ignore_use();
    return;
  }

  // (B3') Like (B6), discard uses of symbols that create 'new' includes.
  if (GlobalFlags().transitive_includes_only) {
    if (!use->has_suggested_header() &&
        !preprocessor_info->FileTransitivelyIncludes(use_file,
                                                     quoted_decl_file)) {
      VERRS(6) << "Ignoring use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc() << "):"
               << " non-transitive #include\n";
      use->set_ignore_use();
      return;
    }
  }
}

void CalculateIwyuForForwardDeclareUse(
    OneUse* use,
    const set<string>& actual_includes,
    const set<string>& desired_includes,
    const set<const FileEntry*>& associated_includes) {
  CHECK_(!use->ignore_use() && "Trying to calculate on an ignored use");
  CHECK_(use->decl() && "CalculateIwyuForForwardDeclareUse takes a fwd-decl");
  CHECK_(!use->is_full_use() && "ForwardDeclareUse are not full uses");

  const NamedDecl* same_file_decl = nullptr;
  const RecordDecl* record_decl = DynCastFrom(use->decl());
  const ClassTemplateDecl* tpl_decl = DynCastFrom(use->decl());
  const ClassTemplateSpecializationDecl* spec_decl = DynCastFrom(use->decl());
  if (spec_decl)
    tpl_decl = spec_decl->getSpecializedTemplate();
  if (tpl_decl)
    record_decl = tpl_decl->getTemplatedDecl();
  CHECK_(record_decl && "Non-records should have been handled already");

  // If this record is defined in one of the desired_includes, mark that
  // fact.  Also if it's defined in one of the actual_includes.
  const NamedDecl* dfn = GetDefinitionForClass(use->decl());
  // If we are, ourselves, a template specialization, then the definition
  // we use is not the definition of the specialization (that's us), but
  // the definition of the template we're specializing.
  if (spec_decl && dfn == spec_decl)
    dfn = GetDefinitionForClass(spec_decl->getSpecializedTemplate());
  bool dfn_is_in_desired_includes = false;
  bool dfn_is_in_actual_includes = false;
  if (dfn) {
    vector<string> headers
      = GlobalIncludePicker().GetCandidateHeadersForFilepathIncludedFrom(
          GetFilePath(dfn), GetFilePath(use->use_loc()));
    for (const string& header : headers) {
      if (ContainsKey(desired_includes, header))
        dfn_is_in_desired_includes = true;
      if (ContainsKey(actual_includes, header))
        dfn_is_in_actual_includes = true;
    }
    // We ourself are always a 'desired' and 'actual' include (though
    // only if the definition is visible from the use location).
    if (DeclIsVisibleToUseInSameFile(dfn, *use)) {
      dfn_is_in_desired_includes = true;
      dfn_is_in_actual_includes = true;
    }
  }

  // We also want to know if *any* redecl of this record is defined
  // in the same file as the use (and before it).
  const set<const NamedDecl*>& redecls = GetClassRedecls(record_decl);
  for (const NamedDecl* redecl : redecls) {
    if (DeclIsVisibleToUseInSameFile(redecl, *use)) {
      same_file_decl = redecl;
      break;
    }
  }
  // If there's no redecl in the .cc file, we'll accept a redecl in
  // an associated .h file.  Since associated .h files are always
  // desired includes, we don't need to check for that.
  if (!same_file_decl) {
    for (const NamedDecl* redecl : redecls) {
      if (ContainsKey(associated_includes, GetFileEntry(redecl))) {
        same_file_decl = redecl;
        break;
      }
    }
  }

  // (D1) Mark that the fwd-declare is satisfied by dfn in desired include.
  const NamedDecl* providing_decl = nullptr;
  if (dfn_is_in_desired_includes) {
    providing_decl = dfn;
    VERRS(6) << "Noting fwd-decl use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << ") is satisfied by dfn in "
             << PrintableLoc(GetLocation(providing_decl)) << "\n";
    // Mark that this use is another reason we want this header.
    const string file = GetFilePath(dfn);
    const string quoted_hdr = ConvertToQuotedInclude(file);
    use->set_suggested_header(quoted_hdr);
  } else if (same_file_decl) {
    providing_decl = same_file_decl;
    VERRS(6) << "Noting fwd-decl use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << ") is declared at "
             << PrintableLoc(GetLocation(providing_decl)) << "\n";
    // If same_file_decl is actually in an associated .h, mark our use
    // of that.  No need to map-to-public for associated .h files.
    if (GetFileEntry(same_file_decl) != GetFileEntry(use->use_loc()))
      use->set_suggested_header(GetFilePath(same_file_decl));
  }
  if (providing_decl) {
    // Change decl_ to point to this "better" redecl.
    use->reset_decl(providing_decl);
  }

  // Be sure to store as a TemplateClassDecl if we're a templated
  // class.
  if (const ClassTemplateSpecializationDecl* spec_decl
      = DynCastFrom(use->decl())) {
    use->reset_decl(spec_decl->getSpecializedTemplate());
  } else if (const CXXRecordDecl* cxx_decl = DynCastFrom(use->decl())) {
    if (cxx_decl->getDescribedClassTemplate())
      use->reset_decl(cxx_decl->getDescribedClassTemplate());
  }

  // (D2) Mark iwyu violation unless defined in a current #include.
  if (dfn_is_in_actual_includes) {
    VERRS(6) << "Ignoring fwd-decl use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): have definition at "
             << PrintableLoc(GetLocation(dfn)) << "\n";
  } else if (same_file_decl) {
    VERRS(6) << "Ignoring fwd-decl use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): have earlier fwd-decl at "
             << PrintableLoc(GetLocation(same_file_decl)) << "\n";
  } else {
    use->set_is_iwyu_violation();
  }
}

void CalculateIwyuForFullUse(OneUse* use,
                             const set<string>& actual_includes,
                             const string& use_quoted_include) {
  CHECK_(!use->ignore_use() && "Trying to calculate on an ignored use");
  CHECK_(use->is_full_use() && "CalculateIwyuForFullUse requires a full use");
  CHECK_(use->has_suggested_header() && "All full uses must have a header");

  // (E1) Discard uses of a symbol declared in a .cc and used
  // elsewhere.  Unless that 'elsewhere' is #including the .cc file,
  // then something is wrong: we're using a symbol from a file we
  // can't possibly be #including.  There are several ways this could
  // happen:
  // (1)
  //   foo.h:  #ifdef FOO ...
  //   foo-inl.cc: #define FOO
  //   foo.cc: #include "foo-inl.cc"
  //           #include "foo.h"   // foo.h 'uses' FOO from foo-inl.cc
  // (Though this is arguably a bug in iwyu, and FOO should be treated as
  // a 'soft' use here; see comments in iwyu_preprocessor.cc:ReportMacroUse.)
  // (2)
  //   foo.h:  #define DEFINE_CLASS(classname) <backslash>
  //             struct classname { classname() { Init(); } void Init() {} }
  //   foo.cc: DEFINE_CLASS(Foo);
  // iwyu will say "Init() is a member function, so say we need the
  // full type information of the method's class."  The method's class
  // is Foo, which iwyu correctly declares lives in foo.cc.  But
  // iwyu also correctly says that Init() lives in foo.h (Except for
  // the macro arguments, macro code belongs to the macro definer,
  // not to every macro caller).  Put those together, though, and
  // iwyu says foo.h needs to #include foo.cc.
  // TODO(csilvers): it's probably more correct to check if
  // suggested_header() is in the transitive closure of actual_includes.
  // TODO(csilvers): this could cause breakage for code like this:
  //    x.cc:    class X {};
  //    y.h:     #include "x.cc"
  //    z.cc:    #include "y.h"; X x;
  // iwyu will say 'replace the #include of y.h with an #include of
  // x.cc,' which the code below will then strip.  The end result is
  // z.cc will not #include anything, and will fail to compile.
  if (!IsHeaderFile(use->suggested_header()) &&
      !ContainsKey(actual_includes, use->suggested_header())) {
    VERRS(6) << "Ignoring use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): #including .cc\n";
    use->set_ignore_use();
    return;
  }

  // (E2) Ignore use when the suggested header *is* the current file (this can
  // happen due to mappings).
  if (use_quoted_include == use->suggested_header()) {
    VERRS(6) << "Ignoring full use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc()
             << "): use already in suggested header "
             << use->suggested_header() << "\n";
    use->set_ignore_use();
    return;
  }

  // (E3) Mark iwyu violation unless in a current #include.
  if (ContainsKey(actual_includes, use->suggested_header())) {
    VERRS(6) << "Ignoring full use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): #including dfn from "
             << use->suggested_header() << "\n";
  } else {
    use->set_is_iwyu_violation();
  }
}

}  // namespace internal

void IwyuFileInfo::CalculateIwyuViolations(vector<OneUse>* uses) {
  VERRS(6) << "--- Calculating IWYU violations for "
           << GetFilePath(file_) << " ---\n";

  // We have to do the steps in order, because a forward-declare use may
  // turn into a full use, and need to be processed in the full-use step
  // too.
  for (OneUse& use : *uses) {
    if (!use.is_full_use() && use.decl())
      internal::ProcessForwardDeclare(&use, preprocessor_info_);
  }
  for (OneUse& use : *uses) {
    if (use.is_full_use() && use.decl())
      internal::ProcessFullUse(&use, preprocessor_info_);
  }
  for (OneUse& use : *uses) {
    if (use.is_full_use() && !use.decl())
      internal::ProcessSymbolUse(&use, preprocessor_info_);
  }

  // (C1) Compute the direct includes of 'associated' files.
  set<string> associated_direct_includes;
  for (const IwyuFileInfo* associated : associated_headers_) {
    ReportIncludeFileUse(associated->file_, associated->quoted_file_);
    InsertAllInto(associated->direct_includes(), &associated_direct_includes);
  }
  // The 'effective' direct includes are defined to be the current
  // includes of associated, plus us.  This is only used to decide
  // when to give iwyu warnings.
  const set<string> effective_direct_includes
      = Union(associated_direct_includes, direct_includes());

  // (C2) + (C3) Find the minimal 'set cover' for all symbol uses.
  const set<string> desired_set_cover = internal::CalculateMinimalIncludes(
      quoted_file_, direct_includes(), associated_direct_includes, uses);

  // (C4) Remove .cc files from desired-includes unless they're in actual-inc.
  for (const string& header_name : desired_set_cover) {
    if (IsHeaderFile(header_name) ||
        ContainsKey(direct_includes(), header_name))
      desired_includes_.insert(header_name);
  }
  desired_includes_have_been_calculated_ = true;

  // The 'effective' desired includes are defined to be the desired
  // includes of associated, plus us.  These are used to decide if a
  // particular use will be satisfied after fixing the #includes.
  // NOTE: this depends on our associated headers having had their
  // iwyu analysis done before us.
  set<string> effective_desired_includes = desired_includes();
  InsertAllInto(AssociatedDesiredIncludes(), &effective_desired_includes);

  // Now that we've figured out desired_includes, figure out iwyu violations.
  for (OneUse& use : *uses) {
    if (use.ignore_use()) {
      // Do nothing, we're ignoring the use
    } else if (!use.is_full_use()) {
      internal::CalculateIwyuForForwardDeclareUse(
          &use, effective_direct_includes, effective_desired_includes,
          AssociatedFileEntries());
    } else {
      internal::CalculateIwyuForFullUse(
          &use, effective_direct_includes, quoted_file_);
    }
  }
}

static string GetWarningMsg(const OneUse& use) {
  const SourceLocation spelling_loc = GetSpellingLoc(use.use_loc());
  const SourceLocation instantiation_loc = GetInstantiationLoc(use.use_loc());
  string warning = PrintableLoc(spelling_loc) + ": warning: ";
  if (use.is_full_use()) {
    warning += (use.symbol_name() + " is defined in " + use.suggested_header()
                + ", which isn't directly #included");
  } else {
    warning += (use.symbol_name() + " needs a declaration"
                + ", but does not provide or directly #include one");
  }
  if (!use.comment().empty()) {
    warning += " " + use.comment();
  }
  warning += ".\n";
  if (instantiation_loc != spelling_loc) {
    // Only set/print this if it's different from the spelling location.
    warning += PrintableLoc(instantiation_loc) + ": note: used here.\n";
  }
  return warning;
}

int IwyuFileInfo::EmitWarningMessages(const vector<OneUse>& uses) {
  set<pair<int, string>> iwyu_warnings;   // line-number, warning-msg.
  for (const OneUse& use : uses) {
    if (use.is_iwyu_violation())
      iwyu_warnings.insert(make_pair(use.UseLinenum(), GetWarningMsg(use)));
  }
  // Nice that set<> automatically sorts things for us!
  for (const pair<int, string>& warning : iwyu_warnings) {
    if (ShouldPrint(3)) {
      errs() << warning.second;
    } else if (ShouldPrint(2)) {
      // TODO(csilvers): print one warning per sym per file.
    }
  }
  return iwyu_warnings.size();
}

namespace internal {

template <class IncludeOrFwdDecl>
bool Contains(const vector<OneIncludeOrForwardDeclareLine>& lines,
              const IncludeOrFwdDecl& item) {
  return std::any_of(lines.begin(), lines.end(),
                     [&](const OneIncludeOrForwardDeclareLine& line) {
    return line.matches(item);
  });
}

template <class ContainerType>
void ClearDesiredForSurplusIncludesOrForwardDeclares(ContainerType& container) {
  // Traverse multimap key by key.
  for (typename ContainerType::iterator k = container.begin();
       k != container.end(); k = container.upper_bound(k->first)) {
    // For each key, mark all but the first value as undesirable.
    typename ContainerType::iterator v = ++container.lower_bound(k->first);
    typename ContainerType::iterator vend = container.upper_bound(k->first);
    for (; v != vend; ++v) {
      v->second->clear_desired();
    }
  }
}

void CalculateDesiredIncludesAndForwardDeclares(
    const vector<OneUse>& uses,
    const set<string>& associated_desired_includes,
    const set<const FileEntry*>& kept_includes,
    vector<OneIncludeOrForwardDeclareLine>* lines) {
  // First make sure all uses' includes and fwd decls are reflected in lines.
  for (const OneUse& use : uses) {
    if (use.ignore_use())
      continue;

    if (use.is_full_use()) {
      CHECK_(use.has_suggested_header() && "Full uses should have #includes");
      if (!Contains(*lines, use.suggested_header())) { // must be added
        lines->push_back(OneIncludeOrForwardDeclareLine(
            use.decl_file(), use.suggested_header(), -1));
      }
    } else if (!use.has_suggested_header()) {
      // Forward-declare uses that are already satisfied by an #include
      // have that as their suggested_header.  For the rest, we need to
      // make sure there's a forward-declare in the current file.
      if (!Contains(*lines, use.decl())) { // must be added
        // The OneIncludeOrForwardDeclareLine ctor sets up line
        // numbers, but they're for some other file!  Clear them.
        lines->push_back(OneIncludeOrForwardDeclareLine(use.decl()));
        lines->back().clear_line_numbers();
      }
    }
  }

  // From this point on, lines is stable and we can refer to its
  // OneIncludeOrForwardDeclareLine objects by pointer.

  // We'll want to be able to map from an include or fwd-declare to the
  // lines where we found them.  There can be multiple includes or fwd-declares
  // providing the same symbol, so use multimaps for these reverse lookups.
  typedef multimap<string, OneIncludeOrForwardDeclareLine*> IncludeMap;
  typedef multimap<const NamedDecl*, OneIncludeOrForwardDeclareLine*>
      FwdDeclMap;

  IncludeMap include_map;
  FwdDeclMap fwd_decl_map;
  for (OneIncludeOrForwardDeclareLine& line : *lines) {
    if (line.IsIncludeLine())
      include_map.insert(std::make_pair(line.quoted_include(), &line));
    else
      fwd_decl_map.insert(std::make_pair(line.fwd_decl(), &line));
  }

  // Now run over all full uses and mark used includes as desired.
  for (const OneUse& use : uses) {
    if (use.ignore_use())
      continue;

    if (use.is_full_use()) {
      // Full uses need a proper include, so mark all corresponding include
      // lines as desired.
      auto range = include_map.equal_range(use.suggested_header());
      for (auto it = range.first; it != range.second; ++it) {
        it->second->set_desired();
        it->second->AddSymbolUse(use.short_symbol_name());
      }
    }
  }

  // Mark forward-decl uses. We need to do this in a separate pass because
  // we need to be sure include_map is fully populated -- we don't want
  // to bother with a "(ptr only)" use if there's already a full use.
  for (const OneUse& use : uses) {
    if (use.ignore_use() || use.is_full_use())
      continue;

    if (!use.has_suggested_header()) {
      // A forward-declare for a use where there is no suggested header to
      // provide the symbol is very much desired.
      auto range = fwd_decl_map.equal_range(use.decl());
      for (auto it = range.first; it != range.second; ++it) {
        it->second->set_desired();
      }
    } else if (ContainsKey(include_map, use.suggested_header())) {
      // If we satisfy a forward-declare use from a file, let the file
      // know (this is just for logging).
      const string symbol_name = use.short_symbol_name();

      auto range = include_map.equal_range(use.suggested_header());
      for (auto it = range.first; it != range.second; ++it) {
        if (!it->second->HasSymbolUse(symbol_name))
          it->second->AddSymbolUse(symbol_name + " (ptr only)");
      }
    }
  }

  // If we #include a .h through an associated file (foo.h) rather
  // than directly (foo.cc), we don't want to say that .h is desired
  // -- that will cause us to add it when it's unnecessary.  We could
  // choose to actually *remove* the .h here if it's present, to keep
  // #includes to a minimum, but for now we just decline to add it.
  for (OneIncludeOrForwardDeclareLine& line : *lines) {
    if (line.is_desired() && !line.is_present() && line.IsIncludeLine() &&
        ContainsKey(associated_desired_includes, line.quoted_include())) {
      line.clear_desired();
    }
  }

  // Clear desired for all duplicates.
  ClearDesiredForSurplusIncludesOrForwardDeclares(include_map);
  ClearDesiredForSurplusIncludesOrForwardDeclares(fwd_decl_map);

  // Now reset all files included with "IWYU pragma: keep" as desired.
  for (OneIncludeOrForwardDeclareLine& line : *lines) {
    if (!line.is_desired() && line.IsIncludeLine() &&
        ContainsKey(kept_includes, line.included_file())) {
      line.set_desired();
    }
  }
}

bool IsRemovablePrefixHeader(const FileEntry* file_entry,
                             const IwyuPreprocessorInfo* preprocessor_info) {
  if (file_entry) {
    IwyuFileInfo* file_info = preprocessor_info->FileInfoFor(file_entry);
    if (file_info)
      return file_info->is_prefix_header() && !file_info->is_pch_in_code();
  }
  return false;
}

void CleanupPrefixHeaderIncludes(
    const IwyuPreprocessorInfo* preprocessor_info,
    vector<OneIncludeOrForwardDeclareLine>* lines) {
  CommandlineFlags::PrefixHeaderIncludePolicy policy =
      GlobalFlags().prefix_header_include_policy;
  if (policy == CommandlineFlags::kAdd)
    return;

  for (OneIncludeOrForwardDeclareLine& line : *lines) {
    if (!line.is_desired())
      continue;
    if (line.is_present() && (policy == CommandlineFlags::kKeep))
      continue;  // Keep present line according to policy.

    const FileEntry* file_entry = nullptr;
    if (line.IsIncludeLine()) {
      file_entry = line.included_file();
      if (!file_entry)
        file_entry = preprocessor_info->IncludeToFileEntry(
            line.quoted_include());
      // At this point it's OK if file_entry is nullptr.  It means we've never
      // seen quoted_include.  And that's why it cannot be prefix header.
    } else {
      const RecordDecl* dfn = GetDefinitionForClass(line.fwd_decl());
      file_entry = GetFileEntry(dfn);
    }
    if (IsRemovablePrefixHeader(file_entry, preprocessor_info)) {
      CHECK_(file_entry && "FileEntry should exist to be prefix header");
      line.clear_desired();
      VERRS(6) << "Ignoring '" << line.line()
               << "': is superseded by command line include "
               << file_entry->getName() << "\n";
    }
  }
}

// Used by GetSymbolsSortedByFrequency().
class CountGt {
 public:
  bool operator()(const pair<string, int>& a,
                  const pair<string, int>& b) const {
    if (a.second != b.second)
      return a.second > b.second;   // sort by decreasing count
    return a.first < b.first;       // within a count, sort alphabetically
  }
};

// Given a map from string to count, creates a vector of the string
// keys, sorted by decreasing count (highest first), then alphabetically.
// We also can take a vector of forward-declared symbols used by this
// file.  For all symbols in this vector but not in m, we add them to
// the end of the output as well, with a "(ptr only)" suffix.
vector<string> GetSymbolsSortedByFrequency(const map<string, int>& m) {
  vector<pair<string, int>> count_vector(m.begin(), m.end());
  sort(count_vector.begin(), count_vector.end(), CountGt());

  vector<string> retval;
  for (const pair<string, int>& count : count_vector)
    retval.push_back(count.first);
  return retval;
}

OutputLine PrintableIncludeOrForwardDeclareLine(
    const OneIncludeOrForwardDeclareLine& line,
    const set<string>& associated_quoted_includes) {
  // If we don't want any comments, always use the line as-is.
  if (GlobalFlags().no_comments) {
    return OutputLine(line.line());
  }

  // Print the line number where we saw this forward-declare or
  // #include, as a comment, if we don't have anything better to show.
  // (For instance, when we want to delete this line.)
  if (line.symbol_counts().empty() && !line.is_present()) {
    return OutputLine(line.line());   // if not present, doesn't have a line #
  }
  if (line.symbol_counts().empty() || !line.is_desired()) {
    CHECK_(!StartsWith(line.LineNumberString(), "-"));
    return OutputLine(line.line() + "  // lines " + line.LineNumberString());
  }
  // We don't need to explain why foo.cc #includes foo.h
  if (line.IsIncludeLine() &&
      ContainsKey(associated_quoted_includes, line.quoted_include())) {
    return OutputLine(line.line());
  }

  return OutputLine(line.line(),
    GetSymbolsSortedByFrequency(line.symbol_counts()));
}

enum class LineSortOrdinal {
  PrecompiledHeader,
  AssociatedHeader,
  AssociatedInlineDefinitions,
  QuotedInclude,
  CHeader,
  CppHeader,
  OtherHeader,
  ForwardDeclaration
};

using LineSortKey = pair<LineSortOrdinal, string>;

LineSortOrdinal GetLineSortOrdinal(const OneIncludeOrForwardDeclareLine& line,
                                   const set<string>& associated_quoted_includes,
                                   const IwyuFileInfo* file_info) {
  if (!line.IsIncludeLine())
    return LineSortOrdinal::ForwardDeclaration;
  if (file_info && file_info->is_pch_in_code())
    return LineSortOrdinal::PrecompiledHeader;

  const std::string quoted_include = line.quoted_include();
  if (ContainsKey(associated_quoted_includes, quoted_include)) {
    if (EndsWith(quoted_include, "-inl.h\""))
      return LineSortOrdinal::AssociatedInlineDefinitions;
    return LineSortOrdinal::AssociatedHeader;
  }

  if (GlobalFlags().quoted_includes_first && EndsWith(quoted_include, "\""))
    return LineSortOrdinal::QuotedInclude;
  if (EndsWith(quoted_include, ".h>"))
    return LineSortOrdinal::CHeader;
  if (EndsWith(quoted_include, ">"))
    return LineSortOrdinal::CppHeader;
  return LineSortOrdinal::OtherHeader;
}

// The sort key of an include/forward-declare line is a (LineSortOrdinal, string)
// pair.  The string is always the line itself.
LineSortKey GetSortKey(const OneIncludeOrForwardDeclareLine& line,
                       const set<string>& associated_quoted_includes,
                       const IwyuFileInfo* file_info) {
  return LineSortKey(GetLineSortOrdinal(line, associated_quoted_includes, file_info), line.line());
}

// filename is "this" filename: the file being emitted.
// associated_filepaths are the quoted-include form of associated_headers_.
size_t PrintableDiffs(const string& filename,
                      const IwyuPreprocessorInfo* preprocessor_info,
                      const set<string>& associated_quoted_includes,
                      const vector<OneIncludeOrForwardDeclareLine>& lines,
                      string* diff_output) {
  CHECK_(diff_output && "Must provide diff_output");

  string& output = *diff_output;
  vector<OutputLine> output_lines;

  const set<string>& aqi = associated_quoted_includes;  // short alias

  // Sort all the output-lines: system headers before user headers
  // before forward-declares, etc.  The easiest way to do this is to
  // just put them all in multimap whose key is a sort-order (multimap
  // because some headers might be listed twice in the source file.)
  multimap<LineSortKey, const OneIncludeOrForwardDeclareLine*> sorted_lines;
  for (const OneIncludeOrForwardDeclareLine& line : lines) {
    const IwyuFileInfo* file_info = nullptr;
    if (line.IsIncludeLine())
      file_info = preprocessor_info->FileInfoFor(line.included_file());

    sorted_lines.insert(make_pair(GetSortKey(line, aqi, file_info), &line));
  }

  // First, check if there are no adds or deletes.  If so, we print a
  // shorter summary line.
  bool no_adds_or_deletes = true;
  for (const auto& key_line : sorted_lines) {
    const OneIncludeOrForwardDeclareLine* line = key_line.second;
    if ((line->is_desired() && !line->is_present()) || // add
        (line->is_present() && !line->is_desired())) { // delete
      no_adds_or_deletes = false;
      break;
    }
  }
  if (no_adds_or_deletes) {
    output = "\n(" + filename + " has correct #includes/fwd-decls)\n";
    return 0;
  }

  size_t num_edits = 0;

  // First, new desired includes and forward-declares.
  if (ShouldPrint(1)) {
    output_lines.push_back(
      OutputLine("\n" + filename + " should add these lines:"));
    for (const auto& key_line : sorted_lines) {
      const OneIncludeOrForwardDeclareLine* line = key_line.second;
      if (line->is_desired() && !line->is_present()) {
        output_lines.push_back(
          PrintableIncludeOrForwardDeclareLine(*line, aqi));
        ++num_edits;
      }
    }
  }

  // Second, includes and forward-declares that should be removed.
  if (ShouldPrint(1)) {
    output_lines.push_back(
        OutputLine("\n" + filename + " should remove these lines:"));
    for (const auto& key_line : sorted_lines) {
      const OneIncludeOrForwardDeclareLine* line = key_line.second;
      if (line->is_present() && !line->is_desired()) {
        output_lines.push_back(
            PrintableIncludeOrForwardDeclareLine(*line, aqi));
        output_lines.back().add_prefix("- ");

        ++num_edits;
      }
    }
  }

  // Finally, print the final, complete include-and-forward-declare list.
  if (ShouldPrint(0)) {
    output_lines.push_back(
      OutputLine("\nThe full include-list for " + filename + ":"));
    for (const auto& key_line : sorted_lines) {
      const OneIncludeOrForwardDeclareLine* line = key_line.second;
      if (line->is_desired()) {
        output_lines.push_back(
          PrintableIncludeOrForwardDeclareLine(*line, aqi));
      }
    }
  }

  // Compute max width of lines with comments so we can align them nicely.
  // This is not strictly necessary if comments have been disabled,
  // but it won't have any effect in that case anyway.
  size_t line_length = 0;
  size_t max_line_length = GlobalFlags().max_line_length;

  for (const OutputLine& line : output_lines) {
    // Only consider lines that need alignment.
    if (line.needs_alignment())
      line_length = std::max(line.line_length(), line_length);
  }

  // Align lines and produce final output.
  for (const OutputLine& line : output_lines) {
    output += line.printable_line(line_length, max_line_length);
    output += "\n";
  }

  // Let's print a helpful separator as well.
  output += "---\n";

  return num_edits;
}

}  // namespace internal

void IwyuFileInfo::HandlePreprocessingDone() {
  // Check macros defined by includer.  Requires file preprocessing to be
  // finished to know all direct includes and all macro usages.
  //
  // Exclude prefix headers from mapping heuristics.  Includes in prefix
  // headers are kept regardless of their usage in includer.  And the entire
  // include-what-you-use principle isn't really applicable to prefix headers.
  if (is_prefix_header()) {
    return;
  }
  bool should_report_violations = ShouldReportIWYUViolationsFor(file_);
  std::list<const FileEntry*> direct_macro_use_includees;
  std::set_intersection(macro_users_.begin(), macro_users_.end(),
                        direct_includes_as_fileentries_.begin(),
                        direct_includes_as_fileentries_.end(),
                        std::inserter(direct_macro_use_includees,
                                      direct_macro_use_includees.end()));
  for (const FileEntry* macro_use_includee : direct_macro_use_includees) {
    if (should_report_violations) {
      ERRSYM(file_) << "Keep #include " << macro_use_includee->getName()
                    << " in " << file_->getName()
                    << " because used macro is defined by includer.\n";
      ReportKnownDesiredFile(macro_use_includee);
    } else {
      string private_include =
          ConvertToQuotedInclude(GetFilePath(macro_use_includee));
      if (GlobalIncludePicker().IsPublic(macro_use_includee)) {
        ERRSYM(file_) << "Skip marking " << quoted_file_
                      << " as public header for " << private_include
                      << " because latter is already marked as public,"
                      << " though uses macro defined by includer.\n";
      } else {
        ERRSYM(file_) << "Mark " << quoted_file_
                      << " as public header for " << private_include
                      << " because used macro is defined by includer.\n";
        MutableGlobalIncludePicker()->AddMapping(
            private_include, MappedInclude(quoted_file_, GetFilePath(file_)));
        MutableGlobalIncludePicker()->MarkIncludeAsPrivate(private_include);
      }
    }
  }
}

void IwyuFileInfo::ResolvePendingAnalysis() {
  // Resolve using declarations:  This handles the case where there's a using
  // declaration in the file but no code is actually using it. If that
  // happens, we might try to remove all of the headers with the decls that
  // the using decl is referencing, which would result in a compilation error
  // at best. A possible solution is to remove the using decl if it's not
  // used, but that doesn't work for header files because a using decl in a
  // header is an exported symbol, so we don't want to do that either. As a
  // compromise, we arbitrarily add the first shadow decl to make sure
  // everything still compiles instead of removing the using decl. A more
  // thorough approach would be to scan the current list of includes that
  // already name this decl (like in the overloaded function case) and include
  // one of those so we don't include a file we don't actually need.
  for (map<const UsingDecl*, bool>::value_type using_decl_status
      : using_decl_referenced_) {
    if (!using_decl_status.second) {
      // There are valid cases where there is no shadow decl, e.g. if a derived
      // class has a using declaration for a member, but also hides it.
      // Only report the target if we have a shadow decl.
      const UsingDecl* using_decl = using_decl_status.first;
      if (using_decl->shadow_size() > 0) {
        ReportForwardDeclareUse(using_decl->getUsingLoc(),
                                using_decl->shadow_begin()->getTargetDecl(),
                                /* flags */ UF_None,
                                "(for un-referenced using)");
      }
    }
  }
}

size_t IwyuFileInfo::CalculateAndReportIwyuViolations() {
  // This is used to calculate our own desired includes.  That depends
  // on what our associated files' desired includes are: if we use
  // bar.h and foo.h is adding it, we don't need to add it ourself.
  // On the other hand, if foo.h used to have it but is removing it,
  // we *do* need to add it.
  set<string> associated_desired_includes = AssociatedDesiredIncludes();

  CalculateIwyuViolations(&symbol_uses_);
  EmitWarningMessages(symbol_uses_);
  internal::CalculateDesiredIncludesAndForwardDeclares(
      symbol_uses_, associated_desired_includes, kept_includes_,  &lines_);

  // Remove desired inclusions that have been inhibited by pragma
  // "no_include".
  for (OneIncludeOrForwardDeclareLine& line : lines_) {
    if (line.IsIncludeLine() &&
        preprocessor_info_->IncludeIsInhibited(file_, line.quoted_include())) {
      line.clear_desired();
    }
  }

  internal::CleanupPrefixHeaderIncludes(preprocessor_info_, &lines_);

  string diff_output;
  size_t num_edits = internal::PrintableDiffs(
      GetFilePath(file_), preprocessor_info_, AssociatedQuotedIncludes(),
      lines_, &diff_output);
  errs() << diff_output;

  return num_edits;
}

}  // namespace include_what_you_use
