//===--- iwyu_output.cpp - output-emitting code for include-what-you-use --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_output.h"
#include <stdio.h>
#include <algorithm>  // for sort(), find()
#include <map>
#include <utility>    // for pair<>
#include <vector>
#include "iwyu_ast_util.h"
#include "iwyu_globals.h"
#include "iwyu_include_picker.h"
#include "iwyu_location_util.h"
#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Type.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceLocation.h"

namespace include_what_you_use {

using clang::ClassTemplateDecl;
using clang::ClassTemplateSpecializationDecl;
using clang::CXXMethodDecl;
using clang::CXXRecordDecl;
using clang::Decl;
using clang::DeclContext;
using clang::FileEntry;
using clang::FullSourceLoc;
using clang::FunctionDecl;
using clang::NamedDecl;
using clang::NamespaceDecl;
using clang::NestedNameSpecifier;
using clang::RecordDecl;
using clang::SourceRange;
using clang::TagDecl;
using clang::TemplateDecl;
using clang::SourceLocation;
using clang::Type;
using llvm::errs;
using llvm::raw_string_ostream;
using std::find;
using std::map;
using std::multimap;
using std::pair;
using std::sort;
using std::vector;

static int FLAGS_verbose = SetVerboseLevel(
    (getenv("IWYU_VERBOSE") == NULL) ? 1 : atoi(getenv("IWYU_VERBOSE")));

int GetVerboseLevel() { return FLAGS_verbose; }
int SetVerboseLevel(int n) {
  FLAGS_verbose = n;
  VERRS(4) << "Setting verbosity level to " << n << "\n";
  return FLAGS_verbose;
}

bool ShouldPrintSymbolFromFile(const FileEntry* file) {
  if (GetVerboseLevel() < 5) {
    return false;
  } else if (GetVerboseLevel() < 10) {
    return ShouldReportIWYUViolationsFor(file);
  } else if (GetVerboseLevel() < 11) {
    return !IsSystemIncludeFile(GetFilePath(file));
  } else {
    return true;
  }
}

namespace internal {

namespace {

// A map that effectively allows us to dynamic cast from a NamedDecl
// to a FakeNamedDecl. When a FakeNamedDecl is created, it will be
// inserted into the map with itself as the key (implicitly casted to
// a NamedDecl).
std::map<const clang::NamedDecl*, const FakeNamedDecl*>
g_fake_named_decl_map;

// Since dynamic casting is not an option, this method is provided to
// determine if a decl is actually a FakeNamedDecl.
const FakeNamedDecl* FakeNamedDeclIfItIsOne(const clang::NamedDecl* decl) {
  return GetOrDefault(g_fake_named_decl_map, decl, NULL);
}

}  // namespace

FakeNamedDecl::FakeNamedDecl(const string& kind_name, const string& qual_name,
                             const string& decl_filepath, int decl_linenum)
    : clang::NamedDecl(clang::Decl::Record, NULL, clang::SourceLocation(),
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
  return tag_decl->getKindName();
}

string GetQualifiedNameAsString(const clang::NamedDecl* named_decl) {
  if (const FakeNamedDecl* fake = FakeNamedDeclIfItIsOne(named_decl)) {
    return fake->qual_name();
  }
  return named_decl->getQualifiedNameAsString();
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
        ostream << "<anonymous " << record_decl->getKindName() << ">::";
      else
        ostream << record_decl << "::";
    } else if (const FunctionDecl *function_decl = DynCastFrom(*it)) {
      ostream << function_decl << "::";   // could also add in '<< "()"'
    } else {
      ostream << cast<NamedDecl>(*it) << "::";
    }
  }
  // Due to the way DeclarationNameInfo::printName() is written, this
  // will show template arguments for templated constructors and
  // destructors.  Since iwyu only shows these when they're defined in
  // a -inl.h file, I'm not going to worry about it.
  if (named_decl->getDeclName())
    ostream << named_decl;
  else
    ostream << "<anonymous>";

  return ostream.str();
}

}  // namespace internal

// Holds information about a single full or fwd-decl use of a symbol.
OneUse::OneUse(const NamedDecl* decl, SourceLocation use_loc,
               OneUse::UseKind use_kind, bool in_cxx_method_body)
    : symbol_name_(internal::GetQualifiedNameAsString(decl)),
      short_symbol_name_(internal::GetShortNameAsString(decl)),
      decl_(decl),
      decl_filepath_(GetFilePath(decl)),
      use_loc_(use_loc),
      use_kind_(use_kind),             // full use or fwd-declare use
      in_cxx_method_body_(in_cxx_method_body),
      public_headers_(),
      suggested_header_(),             // figure that out later
      ignore_use_(false),
      is_iwyu_violation_(false) {
}

// This constructor always creates a full use.
OneUse::OneUse(const string& symbol_name, const string& dfn_filepath,
               SourceLocation use_loc)
    : symbol_name_(symbol_name),
      short_symbol_name_(symbol_name),
      decl_(NULL),
      decl_filepath_(dfn_filepath),
      use_loc_(use_loc),
      use_kind_(kFullUse),
      in_cxx_method_body_(false),
      public_headers_(),
      suggested_header_(),
      ignore_use_(false),
      is_iwyu_violation_(false) {
  // Sometimes dfn_filepath is actually a fully quoted include.  In
  // that case, we take that as an unchangable mapping that we
  // should never remove, so we make it the suggested header.
  assert(!decl_filepath_.empty() && "Must pass a real filepath to OneUse");
  if (decl_filepath_[0] == '"' || decl_filepath_[0] == '<')
    suggested_header_ = decl_filepath_;
}

int OneUse::UseLinenum() const {
  return GetLineNumber(use_loc_);
}

string OneUse::PrintableUseLoc() const {
  return PrintableLoc(use_loc());
}

void OneUse::SetPublicHeaders() {
  const IncludePicker& picker = GlobalIncludePicker();   // short alias
  // If the symbol has a special mapping, use it, otherwise map its file.
  public_headers_ = picker.GetPublicHeadersForSymbol(symbol_name_);
  if (public_headers_.empty())
    public_headers_ = picker.GetPublicHeadersForFilepath(decl_filepath_);
  if (public_headers_.empty())
    public_headers_.push_back(ConvertToQuotedInclude(decl_filepath_));
}

const vector<string>& OneUse::public_headers() {
  if (public_headers_.empty()) {
    SetPublicHeaders();
    assert(!public_headers_.empty() && "Should always have at least one hdr");
  }
  return public_headers_;
}

bool OneUse::PublicHeadersContain(const string& elt) {
  // TODO(csilvers): consider overloading Contains() to handle vectors,
  // and get rid of this method.
  return (std::find(public_headers().begin(), public_headers().end(), elt)
          != public_headers().end());
}

bool OneUse::NeedsSuggestedHeader() const {
  return (!ignore_use() && is_full_use() && suggested_header_.empty());;
}


namespace internal {

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
                           const string& tpl_params_and_kind) {
  // We need to short-circuit the logic for testing.
  if (const FakeNamedDecl* fake = FakeNamedDeclIfItIsOne(decl)) {
    return tpl_params_and_kind + " " + fake->qual_name() + ";";
  }

  assert((isa<RecordDecl>(decl) || isa<TemplateDecl>(decl)) &&
         "IWYU only allows forward declaring (possibly template) record types");

  std::string fwd_decl = std::string(decl->getName()) + ";";
  bool seen_namespace = false;
  for (const DeclContext* ctx = decl->getDeclContext();
       ctx && isa<NamedDecl>(ctx); ctx = ctx->getParent()) {
    if (const RecordDecl* rec = DynCastFrom(ctx)) {
      fwd_decl = std::string(rec->getName()) + "::" + fwd_decl;
    } else if (const NamespaceDecl* ns = DynCastFrom(ctx)) {
      if (!seen_namespace) {
        seen_namespace = true;
        fwd_decl = tpl_params_and_kind + " " + fwd_decl;
      }

      const std::string ns_name = ns->isAnonymousNamespace() ?
          "" : (std::string(ns->getName()) + " ");
      fwd_decl = "namespace " + ns_name + "{ " + fwd_decl + " }";
    } else if (const FunctionDecl* fn = DynCastFrom(ctx)) {
      // A local class (class defined inside a function).
      fwd_decl = std::string(fn->getName()) + "::" + fwd_decl;
    } else {
      assert(false && "Unexpected decoration for type");
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
  return PrintForwardDeclare(decl, GetKindName(decl));
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
  string::size_type endpos = line.length();
  // Get rid of the superclasses, if any (this will nix the body too).
  line = Split(line, " :", 2)[0];
  // Get rid of the template body, if any (true if no superclasses).
  line = Split(line, " {", 2)[0];
  // The template name is now the last word on the line.  Replace it
  // by its fully-qualified form.  Apparently rfind's endpos
  // argument is inclusive, so substract one to get past the end-space.
  const string::size_type name = line.rfind(' ', endpos - 1);
  assert(name != string::npos && "Unexpected printable template-type");
  return PrintForwardDeclare(decl, line.substr(0, name));
}

string MungedForwardDeclareLine(const NamedDecl* decl) {
  if (const RecordDecl* rec_decl = DynCastFrom(decl))
    return MungedForwardDeclareLineForNontemplates(rec_decl);
  else if (const TemplateDecl* template_decl = DynCastFrom(decl))
    return MungedForwardDeclareLineForTemplates(template_decl);
  assert(false && "Unexpected decl type for MungedForwardDeclareLine");
  return "<error>";
}

}  // namespace internal

OneIncludeOrForwardDeclareLine::OneIncludeOrForwardDeclareLine(
    const NamedDecl* fwd_decl)
    : line_(internal::MungedForwardDeclareLine(fwd_decl)),
      start_linenum_(-1), end_linenum_(-1),     // set 'for real' below
      is_desired_(false), is_present_(false), symbol_counts_(),
      quoted_include_(), fwd_decl_(fwd_decl) {
  const SourceRange decl_lines = GetSourceRangeOfClassDecl(fwd_decl);
  start_linenum_ = GetLineNumber(decl_lines.getBegin());
  end_linenum_ = GetLineNumber(decl_lines.getEnd());
}

OneIncludeOrForwardDeclareLine::OneIncludeOrForwardDeclareLine(
    const string& quoted_include, int linenum)
    : line_("#include " + quoted_include),
      start_linenum_(linenum), end_linenum_(linenum),
      is_desired_(false), is_present_(false), symbol_counts_(),
      quoted_include_(quoted_include), fwd_decl_(NULL) {
}

bool OneIncludeOrForwardDeclareLine::HasSymbolUse(const string& symbol_name)
    const {
  return Contains(symbol_counts_, symbol_name);
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


IwyuFileInfo::IwyuFileInfo(const clang::FileEntry* this_file)
  : file_(this_file),
    quoted_file_(ConvertToQuotedInclude(GetFilePath(file_))),
    internal_headers_(),
    symbol_uses_(),
    lines_(),
    direct_includes_(),
    direct_includes_as_fileentries_(),
    direct_forward_declares_(),
    desired_includes_(),
    desired_includes_have_been_calculated_(false)
{}

void IwyuFileInfo::AddInternalHeader(const IwyuFileInfo* other) {
  VERRS(6) << "Adding " << GetFilePath(other->file_)
           << " as internal header for " << GetFilePath(file_) << "\n";
  internal_headers_.insert(other);
}

void IwyuFileInfo::AddInclude(const clang::FileEntry* includee,
                              const string& quoted_includee, int linenumber) {
  lines_.push_back(OneIncludeOrForwardDeclareLine(quoted_includee, linenumber));
  lines_.back().set_present();
  // Store in a few other ways as well.
  direct_includes_as_fileentries_.insert(includee);
  direct_includes_.insert(quoted_includee);
  VERRS(6) << "Marked found include: "
           << GetFilePath(file_) << ":" << linenumber
           << " -> " << GetFilePath(includee) << "\n";
}

void IwyuFileInfo::AddForwardDeclare(const clang::NamedDecl* fwd_decl) {
  assert(fwd_decl && "forward_declare_decl unexpectedly NULL");
  lines_.push_back(OneIncludeOrForwardDeclareLine(fwd_decl));
  lines_.back().set_present();
  direct_forward_declares_.insert(fwd_decl);   // store in another way as well
  VERRS(6) << "Marked found forward-declare: "
           << GetFilePath(file_) << ":" << lines_.back().LineNumberString()
           << ": " << internal::GetQualifiedNameAsString(fwd_decl) << "\n";
}

static void LogSymbolUse(const string& prefix, const OneUse& use) {
  string decl_loc;
  if (use.decl())
    decl_loc = PrintableLoc(GetLocation(use.decl()));
  else
    decl_loc = use.decl_filepath();
  VERRS(6) << prefix << " " << use.symbol_name()
           << " (from " << decl_loc << ")"
           << " at " << use.PrintableUseLoc() << "\n";
}

void IwyuFileInfo::ReportFullSymbolUse(SourceLocation use_loc,
                                       const NamedDecl* decl,
                                       bool in_cxx_method_body) {
  if (decl) {
    // Since we need the full symbol, we need the decl's definition-site.
    decl = GetDefinitionAsWritten(decl);
    symbol_uses_.push_back(OneUse(decl, use_loc, OneUse::kFullUse,
                                  in_cxx_method_body));
    LogSymbolUse("Marked full-info use of decl", symbol_uses_.back());
  }
}

// Converts the type into a decl, and reports that.
void IwyuFileInfo::ReportFullSymbolUse(SourceLocation use_loc,
                                       const Type* type,
                                       bool in_cxx_method_body) {
  if (!type)
    return;
  // The 'full info' for a pointer type can be satisfied by
  // forward-declaring its underlying type.
  if (IsPointerOrReferenceAsWritten(type)) {
    type = RemovePointersAndReferencesAsWritten(type);
    const NamedDecl* decl = TypeToDeclAsWritten(type);
    if (decl) {
      symbol_uses_.push_back(OneUse(decl, use_loc, OneUse::kForwardDeclareUse,
                                    in_cxx_method_body));
      LogSymbolUse("Marked (fwd decl) use of pointer to", symbol_uses_.back());
    }
  } else {
    const NamedDecl* decl = TypeToDeclAsWritten(type);
    if (decl) {
      decl = GetDefinitionAsWritten(decl);
      symbol_uses_.push_back(OneUse(decl, use_loc, OneUse::kFullUse,
                                    in_cxx_method_body));
      LogSymbolUse("Marked full-info use of type", symbol_uses_.back());
    }
  }
}

void IwyuFileInfo::ReportFullSymbolUse(SourceLocation use_loc,
                                       const string& dfn_filepath,
                                       const string& symbol) {
  symbol_uses_.push_back(OneUse(symbol, dfn_filepath, use_loc));
  LogSymbolUse("Marked full-info use of symbol", symbol_uses_.back());
}

void IwyuFileInfo::ReportIncludeFileUse(const string& quoted_include) {
  symbol_uses_.push_back(OneUse("", quoted_include, SourceLocation()));
  LogSymbolUse("Marked use of include-file", symbol_uses_.back());
}


void IwyuFileInfo::ReportForwardDeclareUse(SourceLocation use_loc,
                                           const NamedDecl* decl,
                                           bool in_cxx_method_body) {
  if (!decl)
    return;
  // Sometimes, a bug in clang (http://llvm.org/bugs/show_bug.cgi?id=8669)
  // combines friend decls with true forward-declare decls.  If that
  // happened here, replace the friend with a real fwd decl.
  decl = GetNonfriendClassRedecl(decl);
  symbol_uses_.push_back(OneUse(decl, use_loc, OneUse::kForwardDeclareUse,
                                in_cxx_method_body));
  LogSymbolUse("Marked fwd-decl use of decl", symbol_uses_.back());
}

// Converts the type into a decl, and reports that.
void IwyuFileInfo::ReportForwardDeclareUse(SourceLocation use_loc,
                                           const Type* type,
                                           bool in_cxx_method_body) {
  if (!type)
    return;
  type = RemovePointersAndReferencesAsWritten(type);   // just in case
  const NamedDecl* decl = TypeToDeclAsWritten(type);
  if (decl) {
    decl = GetNonfriendClassRedecl(decl);  // work around clang bug PR#8669
    symbol_uses_.push_back(OneUse(decl, use_loc, OneUse::kForwardDeclareUse,
                                  in_cxx_method_body));
    LogSymbolUse("Marked fwd-decl use of type", symbol_uses_.back());
  }
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

// Helper to tell whether a forward-declare use is 'preceded' by a
// declaration inside the same file.  'Preceded' is in quotes, because
// it's actually ok if the declaration follows the use, inside a
// class.  (You can write a method using a Foo* before defining the
// nested class Foo later in the class.)
bool DeclIsVisibleToUseInSameFile(const Decl* decl, const OneUse& use) {
  if (GetFileEntry(decl) != GetFileEntry(use.use_loc()))
    return false;

  // If the decl comes before the use, it's visible to it.  It can
  // even be visible if the decl comes after, if the decl is inside
  // the class definition and the use is in the body of a method.
  // TODO(csilvers): better than comparing line numbers would be to
  // call SourceManager::getDecomposedLoc() and compare offsets.
  return ((GetInstantiationLineNumber(use.use_loc()) >=
           GetInstantiationLineNumber(GetLocation(decl))) ||
          (DeclsAreInSameClass(decl, use.decl()) && !decl->isOutOfLine()
           && use.in_cxx_method_body()));
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
    const set<string>& direct_includes,
    const set<string>& associated_direct_includes,
    vector<OneUse>* uses) {
  set<string> desired_headers;

  // Step (1) The easy case: decls that map to just one file.  This
  // captures both decls that aren't in private header files, and
  // those in private header files that only map to one public file.
  // For every other decl, we store the (decl, public-headers) pair.
  // Note we can't use Each<> because it only gives const iterators.
  for (vector<OneUse>::iterator it = uses->begin(); it != uses->end(); ++it) {
    // We don't need to add any #includes for non-full-use.
    if (it->ignore_use() || !it->is_full_use())
      continue;
    // Special case #1: Some uses come with a suggested header already picked.
    if (it->has_suggested_header()) {
      desired_headers.insert(it->suggested_header());
      continue;
    }
    // Special case #2: if the dfn-file maps to the use-file, then
    // this is a file that the use-file is re-exporting symbols for,
    // and we should keep the #include as-is.
    const string use_file = ConvertToQuotedInclude(GetFilePath(it->use_loc()));
    if (it->PublicHeadersContain(use_file)) {
      it->set_suggested_header(ConvertToQuotedInclude(it->decl_filepath()));
      desired_headers.insert(it->suggested_header());
      LogIncludeMapping("private header", *it);
    } else if (it->public_headers().size() == 1) {
      it->set_suggested_header(it->public_headers()[0]);
      desired_headers.insert(it->suggested_header());
      LogIncludeMapping("only candidate", *it);
    }
  }

  // Steps (2): Go through the needed private-includes that map to
  // more than one public #include.  First choice: an include in
  // associated_direct_includes (those are includes that are not going
  // away, since we can't change associated files).  Second choice,
  // includes in direct_includes that are also already in
  // desired_headers.  Third choice, includes in desired_headers.
  // Fourth choice, includes in direct_includes.  Picking in
  // this order minimizes the number of #includes we add, while
  // allowing us to remove #includes if need be.
  for (vector<OneUse>::iterator use = uses->begin();
       use != uses->end(); ++use) {
    if (!use->NeedsSuggestedHeader())
      continue;
    const vector<string>& public_headers = use->public_headers();
    // TODO(csilvers): write ElementInBoth() in iwyu_stl_util.h
    for (Each<string> choice(&public_headers);
         !use->has_suggested_header() && !choice.AtEnd(); ++choice) {
      if (Contains(associated_direct_includes, *choice)) {
        use->set_suggested_header(*choice);
        desired_headers.insert(use->suggested_header());
        LogIncludeMapping("in associated header", *use);
      }
    }
    for (Each<string> choice(&public_headers);
         !use->has_suggested_header() && !choice.AtEnd(); ++choice) {
      if (Contains(direct_includes, *choice) &&
          Contains(desired_headers, *choice)) {
        use->set_suggested_header(*choice);
        desired_headers.insert(use->suggested_header());
        LogIncludeMapping("#include already present and needed", *use);
      }
    }
    for (Each<string> choice(&public_headers);
         !use->has_suggested_header() && !choice.AtEnd(); ++choice) {
      if (Contains(desired_headers, *choice)) {
        use->set_suggested_header(*choice);
        desired_headers.insert(use->suggested_header());
        LogIncludeMapping("#include already needed", *use);
      }
    }
    for (Each<string> choice(&public_headers);
         !use->has_suggested_header() && !choice.AtEnd(); ++choice) {
      if (Contains(direct_includes, *choice)) {
        use->set_suggested_header(*choice);
        desired_headers.insert(use->suggested_header());
        LogIncludeMapping("#include already present", *use);
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
  for (vector<OneUse>::iterator it = uses->begin(); it != uses->end(); ++it) {
    if (it->NeedsSuggestedHeader())
      unmapped_uses.insert(&*it);
  }
  while (!unmapped_uses.empty()) {
    map<string, pair<int,int> > header_counts;   // total appearances, 1st's
    for (Each<OneUse*> use(&unmapped_uses); !use.AtEnd(); ++use) {
      assert(!(*use)->has_suggested_header());
      const vector<string>& public_headers = (*use)->public_headers();
      for (Each<string> choice(&public_headers);
           !(*use)->has_suggested_header() && !choice.AtEnd(); ++choice) {
        ++header_counts[*choice].first;     // increment total count
        if (*choice == (*use)->public_headers()[0])
          ++header_counts[*choice].second;  // increment first-in-list count
      }
    }
    pair<string, pair<int, int> > best = *header_counts.begin();
    for (Each<string, pair<int, int> > it(&header_counts); !it.AtEnd(); ++it) {
      if (it->second > best.second)  // uses pair<>'s operator> to order for us
        best = *it;
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
// A3) If a nested class, discard this use (the parent class declaration
//     is sufficient).
// A4) If any of the redeclarations of this declaration is in the same
//     file as the use (and before it), and is actually a definition,
//     discard the forward-declare use.

// Trimming symbol uses (1st pass):
// B1) Discard symbol uses of a symbol defined in the same file it's used.
// B2) Discard symbol uses for builtin symbols ('__builtin_memcmp') and
//     for operator new and operator delete (excluding placement new),
//     which are effectively built-in even though they're in <new>.
// B3) Discard .h-file uses of symbols defined in a .cc file (sanity check).
// B4) Discard symbol uses for member functions that live in the same
//     file as the class they're part of (the parent check suffices).
// B5) Discard macro uses in the same file as the definition (B1 redux).
// B6) Discard .h-file uses of macros defined in a .cc file (B3 redux).

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
// E1) If the desired include-file for this symbols is not in the
//     current includes, mark as an iwyu violation.

void ProcessForwardDeclare(OneUse* use) {
  assert(use->decl() && "Must call ProcessForwardDeclare on a decl");
  assert(!use->is_full_use() && "Must call ProcessForwardDeclare on fwd-decl");
  if (use->ignore_use())   // we're already ignoring it
    return;

  // (A1) If not a class or a templated class, recategorize as a full use.
  const RecordDecl* record_decl = DynCastFrom(use->decl());
  if (const ClassTemplateDecl* tpl_decl = DynCastFrom(use->decl()))
    record_decl = tpl_decl->getTemplatedDecl();
  if (!record_decl) {
    VERRS(6) << "Moving " << use->symbol_name()
             << " from fwd-decl use to full use: not a class"
             << " (" << use->PrintableUseLoc() << ")\n";
    use->set_full_use();
    return;
  }

  // (A2) If it has default template parameters, recategorize as a full use.
  if (const ClassTemplateDecl* tpl_decl = DynCastFrom(use->decl())) {
    if (HasDefaultTemplateParameters(tpl_decl)) {
      VERRS(6) << "Moving " << use->symbol_name()
               << " from fwd-decl use to full use: has default template param"
               << " (" << use->PrintableUseLoc() << ")\n";
      use->set_full_use();
      // No return here: (A3) or (A4) may cause us to ignore this decl entirely.
    }
  }

  // (A3) If a nested class, discard this use.
  if (record_decl->getQualifier() &&
      record_decl->getQualifier()->getKind() == NestedNameSpecifier::TypeSpec) {
    // iwyu will require the full type of the parent class when it
    // recurses on the qualifier (any use of Foo::Bar requires the
    // full type of Foo).  So if we're forward-declared inside Foo,
    // the user will get that forward-declaration for free when
    // it gets the full definition of Foo.
    VERRS(6) << "Ignoring fwd-decl use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): nested class\n";
    use->set_ignore_use();
    return;
  }

  // (A4) If a definition exists earlier in this file, discard this use.
  // Note: for the 'earlier' checks, what matters is the *instantiation*
  // location.
  const set<const RecordDecl*> redecls = GetClassRedecls(record_decl);
  for (Each<const RecordDecl*> it(&redecls); !it.AtEnd(); ++it) {
    const SourceLocation defined_loc = GetLocation(*it);
    if ((*it)->isDefinition() && DeclIsVisibleToUseInSameFile(*it, *use)) {
      VERRS(6) << "Ignoring fwd-decl use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc() << "): dfn is present: "
               << PrintableLoc(defined_loc) << "\n";
      use->set_ignore_use();
      return;
    }
  }
}

void ProcessFullUse(OneUse* use) {
  assert(use->decl() && "Must call ProcessFullUse on a decl");
  assert(use->is_full_use() && "Must not call ProcessFullUse on fwd-decl");
  if (use->ignore_use())   // we're already ignoring it
    return;

  // (B1) Discard symbol uses of a symbol defined in the same file it's used.
  if (GetFileEntry(use->use_loc()) == GetFileEntry(use->decl())) {
    VERRS(6) << "Ignoring use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): definition is present: "
             << PrintableLoc(GetLocation(use->decl())) << "\n";
    use->set_ignore_use();
    return;
  }

  // (B2) Discard symbol uses for builtin symbols, including new/delete.
  // TODO(csilvers): we could use getBuiltinID(), but it returns
  // non-zero for things like malloc.  Figure out how to use it.
  if (const FunctionDecl* fn_decl = DynCastFrom(use->decl())) {
    if (StartsWith(use->symbol_name(), "__builtin_")) {
      VERRS(6) << "Ignoring use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc() << "): built-in function\n";
      use->set_ignore_use();
      return;
    }
    const string dfn_file = GetFilePath(fn_decl);
    if (IsDefaultNewOrDelete(fn_decl, ConvertToQuotedInclude(dfn_file))) {
      VERRS(6) << "Ignoring use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc() << "): built-in new/delete\n";
      use->set_ignore_use();
      return;
    }
  }

  // (B3) Discard uses of a symbol declared in a .cc and used in a .h.
  // Here's how it could happen:
  //   foo.h:  #define DEFINE_CLASS(classname) <backslash>
  //             struct classname { classname() { Init(); } void Init() {} };
  //   foo.cc: DEFINE_CLASS(Foo).
  // iwyu will say "Init() is a member function, so say we need the
  // full type information of the parent class."  The parent class
  // is Foo, which iwyu correctly declares lives in foo.cc.  But
  // iwyu also correctly says that Init() lives in foo.h (Except for
  // the macro arguments, macro code belongs to the macro definer,
  // not to every macro caller).  Put those together, though, and
  // iwyu says foo.h needs to #include foo.cc.
  // TODO(csilvers): instead of checking file extensions, check if
  // defined_in is in the transitive closure of used_in's #includes.
  if (use->use_loc().isValid() && IsHeaderFile(GetFilePath(use->use_loc())) &&
      !IsHeaderFile(use->decl_filepath())) {
    VERRS(6) << "Ignoring use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): .h #including .cc\n";
    use->set_ignore_use();
    return;
  }

  // (B4) Discard symbol uses for member functions in the same file as parent.
  if (const CXXMethodDecl* method_dfn = DynCastFrom(use->decl())) {
    // See if we also recorded a use of the parent.
    const NamedDecl* parent_dfn
        = GetDefinitionAsWritten(method_dfn->getParent());
    // We want to map the definition-files to their public headers if
    // they're private headers (so bits/stl_vector.h and
    // bits/vector.tcc are counted as the "same" file for this test).
    // To be safe, we only do the mapping if both files have at most
    // one public file they map to (otherwise we don't know which
    // mapping to choose, and it's important we use the one that iwyu
    // will pick later).  TODO(csilvers): figure out that case too.
    const IncludePicker& picker = GlobalIncludePicker();
    const vector<string>& method_dfn_files
        = picker.GetPublicHeadersForFilepath(GetFilePath(method_dfn));
    const vector<string>& parent_dfn_files
        = picker.GetPublicHeadersForFilepath(GetFilePath(parent_dfn));
    bool same_file;
    if (method_dfn_files.size() == 1 && parent_dfn_files.size() == 1) {
      same_file = (method_dfn_files[0] == parent_dfn_files[0]);
    } else {
      // Fall back on just checking the filenames: can't figure out public.
      same_file = (GetFileEntry(method_dfn) == GetFileEntry(parent_dfn));
    }
    if (same_file) {
      VERRS(6) << "Ignoring use of " << use->symbol_name()
               << " (" << use->PrintableUseLoc() << "): member of class\n";
      use->set_ignore_use();
      return;
    }
  }
}

void ProcessSymbolUse(OneUse* use) {
  if (use->ignore_use())   // we're already ignoring it
    return;

  // (B5) Discard symbol uses in the same file as their definition.
  if (GetFilePath(use->use_loc()) == use->decl_filepath()) {
    VERRS(6) << "Ignoring symbol use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): defined in same file\n";
    use->set_ignore_use();
    return;
  }

  // (B6) Discard uses of a macro declared in a .cc and used in a .h.
  // Here's how it could happen:
  //   foo.h:  #ifdef FOO ...
  //   foo-inl.cc: #define FOO
  //   foo.cc: #include "foo-inl.cc"
  //           #include "foo.h"
  if (use->use_loc().isValid() && IsHeaderFile(GetFilePath(use->use_loc())) &&
      !IsHeaderFile(use->decl_filepath())) {
    VERRS(6) << "Ignoring symbol use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << "): "
             << "avoid .h #including .cc\n";
    use->set_ignore_use();
    return;
  }
}

void CalculateIwyuForForwardDeclareUse(
    OneUse* use,
    const set<string>& actual_includes,
    const set<string>& desired_includes,
    const set<const FileEntry*>& associated_includes) {
  assert(!use->ignore_use() && "Trying to calculate on an ignored use");
  assert(use->decl() && "CalculateIwyuForForwardDeclareUse takes a fwd-decl");
  assert(!use->is_full_use() && "ForwardDeclareUse are not full uses");

  // If this record is defined in one of the desired_includes, mark that
  // fact.  Also if it's defined in one of the actual_includes.
  const RecordDecl* dfn = GetDefinitionForClass(use->decl());
  bool dfn_is_in_desired_includes = false;
  bool dfn_is_in_actual_includes = false;
  if (dfn) {
    vector<string> headers
        = GlobalIncludePicker().GetPublicHeadersForFilepath(GetFilePath(dfn));
    for (Each<string> header(&headers); !header.AtEnd(); ++header) {
      if (Contains(desired_includes, *header))
        dfn_is_in_desired_includes = true;
      if (Contains(actual_includes, *header))
        dfn_is_in_actual_includes = true;
    }
  }

  // We also want to know if *any* redecl of this record is defined
  // in the same file as the use (and before it).
  const NamedDecl* same_file_decl = NULL;
  const RecordDecl* record_decl = DynCastFrom(use->decl());
  const ClassTemplateDecl* tpl_decl = DynCastFrom(use->decl());
  if (tpl_decl)
    record_decl = tpl_decl->getTemplatedDecl();
  assert(record_decl && "Non-records should have been handled already");
  const set<const RecordDecl*>& redecls = GetClassRedecls(record_decl);
  for (Each<const RecordDecl*> it(&redecls); !it.AtEnd(); ++it) {
    if (DeclIsVisibleToUseInSameFile(*it, *use)) {
      same_file_decl = *it;
      break;
    }
  }
  // If there's no redecl in the .cc file, we'll accept a redecl in
  // an associated .h file.  Since associated .h files are always
  // desired includes, we don't need to check for that.
  if (!same_file_decl) {
    for (Each<const RecordDecl*> it(&redecls); !it.AtEnd(); ++it) {
      if (Contains(associated_includes, GetFileEntry(*it))) {
        same_file_decl = *it;
        break;
      }
    }
  }

  // (D1) Mark that the fwd-declare is satisfied by dfn in desired include.
  const NamedDecl* providing_decl = NULL;
  if (dfn_is_in_desired_includes) {
    VERRS(6) << "Noting fwd-decl use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << ") is satisfied by dfn in "
             << PrintableLoc(GetLocation(providing_decl)) << "\n";
    providing_decl = dfn;
    // Mark that this use is another reason we want this header.
    const string file = GetFilePath(dfn);
    const string quoted_hdr = ConvertToQuotedInclude(file);
    use->set_suggested_header(quoted_hdr);
  } else if (same_file_decl) {
    VERRS(6) << "Noting fwd-decl use of " << use->symbol_name()
             << " (" << use->PrintableUseLoc() << ") is declared at "
             << PrintableLoc(GetLocation(providing_decl)) << "\n";
    providing_decl = same_file_decl;
    // If same_file_decl is actually in an associated .h, mark our use
    // of that.  No need to map-to-public for associated .h files.
    if (GetFileEntry(same_file_decl) != GetFileEntry(use->use_loc()))
      use->set_suggested_header(GetFilePath(same_file_decl));
  }
  if (providing_decl) {
    // Change decl_ to point to this "better" redecl.  Be sure to store
    // as a TemplateClassDecl if that's what decl_ was originally.
    if (tpl_decl) {
      assert(isa<CXXRecordDecl>(providing_decl) &&
             cast<CXXRecordDecl>(providing_decl)->getDescribedClassTemplate());
      const CXXRecordDecl* cxx_decl = cast<CXXRecordDecl>(providing_decl);
      use->reset_decl(cxx_decl->getDescribedClassTemplate());
    } else {
      use->reset_decl(providing_decl);
    }
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
                             const set<string>& desired_includes) {
  assert(!use->ignore_use() && "Trying to calculate on an ignored use");
  assert(use->is_full_use() && "CalculateIwyuForFullUse requires a full use");
  assert(use->has_suggested_header() && "All full uses must have a header");

  // (E1) Mark iwyu violation unless in a current #include.
  if (Contains(actual_includes, use->suggested_header())) {
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
  // too.  Note we can't use Each<> because it returns a const-iterator.
  for (vector<OneUse>::iterator it = uses->begin(); it != uses->end(); ++it) {
    if (!it->is_full_use() && it->decl())
      internal::ProcessForwardDeclare(&*it);
  }
  for (vector<OneUse>::iterator it = uses->begin(); it != uses->end(); ++it) {
    if (it->is_full_use() && it->decl())
      internal::ProcessFullUse(&*it);
  }
  for (vector<OneUse>::iterator it = uses->begin(); it != uses->end(); ++it) {
    if (it->is_full_use() && !it->decl())
      internal::ProcessSymbolUse(&*it);
  }

  // (C1) Compute the direct includes of 'associated' files.
  set<string> associated_direct_includes;
  for (Each<const IwyuFileInfo*> it(&internal_headers_); !it.AtEnd(); ++it) {
    ReportIncludeFileUse((*it)->quoted_file_);
    InsertAllInto((*it)->direct_includes(), &associated_direct_includes);
  }
  // The 'effective' direct includes are defined to be the current
  // includes of associated, plus us.  This is only used to decide
  // when to give iwyu warnings.
  const set<string> effective_direct_includes
      = Union(associated_direct_includes, direct_includes());

  // (C2) + (C3) Find the minimal 'set cover' for all symbol uses.
  desired_includes_ = internal::CalculateMinimalIncludes(
      direct_includes(), associated_direct_includes, uses);
  desired_includes_have_been_calculated_ = true;

  // The 'effective' desired includes are defined to be the desired
  // includes of associated, plus us.  These are used to decide if a
  // particular use will be satisfied after fixing the #includes.
  // NOTE: this depends on our internal headers having had their
  // iwyu analysis done before us.
  set<string> effective_desired_includes = desired_includes();
  for (Each<const IwyuFileInfo*> it(&internal_headers_); !it.AtEnd(); ++it)
    InsertAllInto((*it)->desired_includes(), &effective_desired_includes);

  // Now that we've figured out desired_includes, figure out iwyu violations.
  for (vector<OneUse>::iterator it = uses->begin(); it != uses->end(); ++it) {
    if (it->ignore_use()) {
      // Do nothing, we're ignoring the use
    } else if (!it->is_full_use()) {
      internal::CalculateIwyuForForwardDeclareUse(
          &*it, effective_direct_includes, effective_desired_includes,
          AssociatedFileEntries());
    } else {
      internal::CalculateIwyuForFullUse(
          &*it, effective_direct_includes, effective_desired_includes);
    }
  }
}

static string GetWarningMsg(const OneUse& use) {
  const FullSourceLoc spelling_loc = GetSpellingLoc(use.use_loc());
  const FullSourceLoc instantiation_loc = GetInstantiationLoc(use.use_loc());
  string warning = PrintableLoc(spelling_loc) + ": warning: ";
  if (use.is_full_use()) {
    warning += (use.symbol_name() + " is defined in " + use.suggested_header()
                + ", which isn't directly #included.\n");
  } else {
    warning += (use.symbol_name() + " needs a declaration"
                + ", but does not provide or directly #include one.\n");
  }
  if (instantiation_loc != spelling_loc) {
    // Only set/print this if it's different from the spelling location.
    warning += PrintableLoc(instantiation_loc) + ": note: used here.\n";
  }
  return warning;
}

int IwyuFileInfo::EmitWarningMessages(const vector<OneUse>& uses) {
  set<pair<int, string> > iwyu_warnings;   // line-number, warning-msg.
  for (Each<OneUse> it(&uses); !it.AtEnd(); ++it) {
    if (it->is_iwyu_violation())
      iwyu_warnings.insert(make_pair(it->UseLinenum(), GetWarningMsg(*it)));
  }
  // Nice that set<> automatically sorts things for us!
  for (Each<pair<int, string> > it(&iwyu_warnings); !it.AtEnd(); ++it) {
    if (ShouldPrint(3)) {
      errs() << it->second;
    } else if (ShouldPrint(2)) {
      // TODO(csilvers): print one warning per sym per file.
    }
  }
  return iwyu_warnings.size();
}

namespace internal {
void CalculateDesiredIncludesAndForwardDeclares(
    const vector<OneUse>& uses,
    const set<string>& associated_desired_includes,
    vector<OneIncludeOrForwardDeclareLine>* lines) {
  // We'll want to be able to map from decl or fwd-declare to the
  // line where we found it.  We store an index into lines.
  map<string, int> include_map;
  map<const NamedDecl*, int> fwd_decl_map;
  int index = 0;
  for (Each<OneIncludeOrForwardDeclareLine> it(lines); !it.AtEnd(); ++it) {
    if (it->IsIncludeLine())
      include_map[it->quoted_include()] = index;
    else
      fwd_decl_map[it->fwd_decl()] = index;
    ++index;
  }

  for (Each<OneUse> use(&uses); !use.AtEnd(); ++use) {
    if (use->ignore_use())
      continue;
    // Update the appropriate map depending on the type of use.
    if (use->is_full_use()) {
      assert(use->has_suggested_header() && "Full uses should have #includes");
      if (!Contains(include_map, use->suggested_header())) {  // must be added
        lines->push_back(OneIncludeOrForwardDeclareLine(use->suggested_header(),
                                                        -1));
        include_map[use->suggested_header()] = lines->size() - 1;
      }
      const int index = include_map[use->suggested_header()];
      (*lines)[index].set_desired();
      (*lines)[index].AddSymbolUse(use->short_symbol_name());

    // Forward-declare uses that are already satisfied by an #include
    // have that as their suggested_header.  For the rest, we need to
    // make sure there's a forward-declare in the current file.
    } else if (!use->has_suggested_header()) {
      if (!Contains(fwd_decl_map, use->decl())) {  // must be added
        lines->push_back(OneIncludeOrForwardDeclareLine(use->decl()));
        // The OneIncludeOrForwardDeclareLine ctor sets up line
        // numbers, but they're for some other file!  Clear them.
        lines->back().clear_line_numbers();
        fwd_decl_map[use->decl()] = lines->size() - 1;
      }
      const int index = fwd_decl_map[use->decl()];
      (*lines)[index].set_desired();
    }
  }

  // If we satisfy a forward-declare use from a file, let the file
  // know (this is just for logging).  We do this after the above so
  // we can make sure include_map is fully populated -- we don't want
  // to bother with a "(ptr only)" use if there's already a full use.
  for (Each<OneUse> use(&uses); !use.AtEnd(); ++use) {
    if (!use->ignore_use() && !use->is_full_use() && use->has_suggested_header()
        && Contains(include_map, use->suggested_header())) {
      const string symbol_name = use->short_symbol_name();
      const int index = include_map[use->suggested_header()];
      if (!(*lines)[index].HasSymbolUse(symbol_name))
        (*lines)[index].AddSymbolUse(symbol_name + " (ptr only)");
    }
  }

  // If we #include a .h through an associated file (foo.h) rather
  // than directly (foo.cc), we don't want to say that .h is desired
  // -- that will cause us to add it when it's unnecessary.  We could
  // choose to actually *remove* the .h here if it's present, to keep
  // #includes to a minimum, but for now we just decline to add it.
  for (vector<OneIncludeOrForwardDeclareLine>::iterator
           it = lines->begin(); it != lines->end(); ++it) {
    if (it->is_desired() && !it->is_present() && it->IsIncludeLine() &&
        Contains(associated_desired_includes, it->quoted_include())) {
      it->clear_desired();
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
  vector<pair<string, int> > count_vector(m.begin(), m.end());
  sort(count_vector.begin(), count_vector.end(), CountGt());

  vector<string> retval;
  for (Each<pair<string, int> > i(&count_vector); !i.AtEnd(); ++i)
    retval.push_back(i->first);
  return retval;
}

// A helper function that returns one line of the desired-includes blobs.
string PrintableIncludeOrForwardDeclareLine(
    const OneIncludeOrForwardDeclareLine& line,
    const set<string>& associated_quoted_includes) {
  // Print the line number where we saw this forward-declare or
  // #include, as a comment, if we don't have anything better to show.
  // (For instance, when we want to delete this line.)
  if (line.symbol_counts().empty() && !line.is_present()) {
    return line.line() + "\n";   // if not present, doesn't have a line #
  }
  if (line.symbol_counts().empty() || !line.is_desired()) {
    assert(!StartsWith(line.LineNumberString(), "-"));
    return line.line() + "  // lines " + line.LineNumberString() + "\n";
  }
  // We don't need to explain why foo.cc #includes foo.h
  if (line.IsIncludeLine() &&
      Contains(associated_quoted_includes, line.quoted_include())) {
    return line.line() + "\n";
  }
  string retval = line.line();
  string prefix;   // what we print before each symbol in the 'why' comments
  // We try to get the columns to line up nicely.  The 38 is arbitrary.
  if (retval.length() < 38)
    prefix += string(38 - retval.length(), ' ');
  prefix += "  // for ";    // before 1st symbol, print ' // for '
  int symbols_printed = 0;
  vector<string> symbols(GetSymbolsSortedByFrequency(line.symbol_counts()));
  for (Each<string> it(&symbols); !it.AtEnd(); ++it) {
    if (it->empty())       // ignore the empty ("") symbol
      continue;
    // At verbose levels of 0, 1, or 2, cut off output at 80 columns.
    // Actually, at 74, to leave 5 chars for ', etc' and 1 for newline.
    if (ShouldPrint(3) ||
        retval.length() + prefix.length() + it->length() <= 74) {
      retval += prefix + *it;
      ++symbols_printed;
      prefix = ", ";       // before 2nd and subsequent symbols, print ', '
    } else {
      // Truncate at 80 cols.
      if (symbols_printed > 0)
        retval += ", etc";
      break;
    }
  }
  retval += "\n";
  return retval;
}

typedef pair<int, string> LineSortKey;

// The sort key of an include/forward-declare line is an (int, string)
// pair.  The string is always the line itself.  The int is a category:
// 1: associated .h, 2: associated -inl.h, 3: C header, 4: c++ header,
// 5: other header, 6: forward-declare.
LineSortKey GetSortKey(const OneIncludeOrForwardDeclareLine& line,
                       const set<string>& associated_quoted_includes) {
  if (!line.IsIncludeLine())
    return LineSortKey(6, line.line());
  if (Contains(associated_quoted_includes, line.quoted_include())) {
    if (EndsWith(line.quoted_include(), "-inl.h\""))
      return LineSortKey(2, line.line());
    return LineSortKey(1, line.line());
  }
  if (EndsWith(line.quoted_include(), ".h>"))
    return LineSortKey(3, line.line());
  if (EndsWith(line.quoted_include(), ">"))
    return LineSortKey(4, line.line());
  return LineSortKey(5, line.line());
}

// filename is "this" filename: the file being emitted.
// associated_filepaths are the quoted-include form of internal_headers_.
string PrintableDiffs(const string& filename,
                      const set<string>& associated_quoted_includes,
                      const vector<OneIncludeOrForwardDeclareLine>& lines) {
  const set<string>& aqi = associated_quoted_includes;  // short alias

  // Sort all the output-lines: system headers before user headers
  // before forward-declares, etc.  The easiest way to do this is to
  // just put them all in multimap whose key is a sort-order (multimap
  // because some headers might be listed twice in the source file.)
  multimap<LineSortKey, const OneIncludeOrForwardDeclareLine*> sorted_lines;
  for (Each<OneIncludeOrForwardDeclareLine> it(&lines); !it.AtEnd(); ++it) {
    sorted_lines.insert(make_pair(GetSortKey(*it, aqi), &*it));
  }

  // First, check if there are no adds or deletes.  If so, we print a
  // shorter summary line.
  bool no_adds_or_deletes = true;
  for (Each<LineSortKey, const OneIncludeOrForwardDeclareLine*>
           it(&sorted_lines); !it.AtEnd(); ++it) {
    if ((it->second->is_desired() && !it->second->is_present()) ||  // add
        (it->second->is_present() && !it->second->is_desired())) {  // delete
      no_adds_or_deletes = false;
      break;
    }
  }
  if (no_adds_or_deletes) {
    return "\n(" + filename + " has correct #includes/fwd-decls)\n";
  }

  string output;
  // First, new desired includes and forward-declares.
  if (ShouldPrint(1)) {
    output += "\n" + filename + " should add these lines:\n";
    for (Each<LineSortKey, const OneIncludeOrForwardDeclareLine*>
             it(&sorted_lines); !it.AtEnd(); ++it) {
      if (it->second->is_desired() && !it->second->is_present()) {
        output += PrintableIncludeOrForwardDeclareLine(*it->second, aqi);
      }
    }
  }

  // Second, includes and forward-declares that should be removed.
  if (ShouldPrint(1)) {
    output += "\n" + filename + " should remove these lines:\n";
    for (Each<LineSortKey, const OneIncludeOrForwardDeclareLine*>
             it(&sorted_lines); !it.AtEnd(); ++it) {
      if (it->second->is_present() && !it->second->is_desired()) {
        output += "- " + PrintableIncludeOrForwardDeclareLine(*it->second, aqi);
      }
    }
  }

  // Finally, print the final, complete include-and-forward-declare list.
  if (ShouldPrint(0)) {
    output += "\nThe full include-list for " + filename + ":\n";
    for (Each<LineSortKey, const OneIncludeOrForwardDeclareLine*>
             it(&sorted_lines); !it.AtEnd(); ++it) {
      if (it->second->is_desired()) {
        output += PrintableIncludeOrForwardDeclareLine(*it->second, aqi);
      }
    }
  }

  // Let's print a helpful separator as well.
  output += "---\n";

  return output;
}

}  // namespace internal

void IwyuFileInfo::EmitDiffs(
    const vector<OneIncludeOrForwardDeclareLine>& lines) {
  errs() << internal::PrintableDiffs(GetFilePath(file_),
                                     AssociatedQuotedIncludes(),
                                     lines);
}

int IwyuFileInfo::CalculateAndReportIwyuViolations() {
  // This is used to calculate our own desired includes.  That depends
  // on what our associated files' desired includes are: if we use
  // bar.h and foo.h is adding it, we don't need to add it ourself.
  // On the other hand, if foo.h used to have it but is removing it,
  // we *do* need to add it.
  set<string> associated_desired_includes;
  for (Each<const IwyuFileInfo*> it(&internal_headers_); !it.AtEnd(); ++it) {
    InsertAllInto((*it)->desired_includes(), &associated_desired_includes);
  }

  CalculateIwyuViolations(&symbol_uses_);
  const int retval = EmitWarningMessages(symbol_uses_);
  internal::CalculateDesiredIncludesAndForwardDeclares(
      symbol_uses_, associated_desired_includes, &lines_);
  EmitDiffs(lines_);
  return retval;
}

}  // namespace include_what_you_use
