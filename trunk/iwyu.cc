//===--- iwyu.cpp - main logic and driver for include-what-you-use --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A Clang-based tool that catches Include-What-You-Use violations:
//
// The analysis enforces the following rule:
//
// - For every symbol (variable, function, constant, type, and macro)
//   X in C++ file CU.cc, X must be declared in CU.cc or in a header
//   file directly included by itself, CU.h, or CU-inl.h.  Likewise
//   for CU.h and CU-inl.h.
//
// The rule has a few slight wrinkles:
// 1) CU_test.cc can also 'inherit' #includes from CU.h and CU-inl.h.
//    Likewise for CU_unittest.cc, CU_regtest.cc, etc.
// 2) CU.cc can inherit #includes from ../public/CU.h in addition to
//    ./CU.h (likewise for -inl.h).
// 3) For system #includes, and a few google #includes, we hard-code
//    in knowledge of which #include files are public and which are not.
//    (For instance, <vector> is public, <bits/stl_vector.h> is not.)
//    We force CU.cc, CU.h, and CU-inl.h to #include the public version.
//
// iwyu.cc checks if a symbol can be forward-declared instead of fully
// declared.  If so, it will enforce the rule that the symbol is
// forward-declared in the file that references it.  We only forward-
// declare classes and structs (possibly templatized).  We will not
// try to forward-declare variables or functions.
//
// Checking iwyu violations for variables, functions, constants, and
// macros is easy.  Types can be trickier.  Obviously, if you declare
// a variable of type Foo in cu.cc, it's straightforward to check
// whether it's an iwyu violation.  But what if you call a function
// that returns a type, e.g. 'if (FnReturningSomeSTLType()->empty())'?
// Is it an iwyu violation if you don't #include the header for that
// STL type?  We say no: whatever file provided the function
// FnReturningSomeSTLType is also responsible for providing whatever
// the STL type is, so we don't have to.  Otherwise, we get into an
// un-fun propagation problem if we change the signature of
// FnReturningSomeSTLType to return a different type of STL type.  So
// in general, types are only iwyu-checked if they appear explicitly
// in the source code.
//
// It can likewise be difficult to say whether a template arg is
// forward-declable: set<Foo*> x does not require the full type info
// for Foo, but remove_pointer<Foo*>::type does.  And f<Foo>() doesn't
// require full type info for Foo if f doesn't actually use Foo in it.
// For now we do the simple heuristic that if the template arg is a
// pointer, it's ok if it's forward-declared, and if not, it's not.
//
// We use the following terminology:
//
//   - A *symbol* is the name of a function, variable, constant, type,
//     macro, etc.
//
//   - A *quoted include path* is an include path with the surrounding <>
//     or "", e.g. <stdio.h> or "ads/util.h".
//
//   - Any #include falls into exactly one of three (non-overlapping)
//     categories:
//
//     * it's said to be *necessary* if it's a compiler or IWYU error to
//       omit the #include;
//
//     * it's said to be *optional* if the #include is unnecessary but
//       having it is not an IWYU error either (e.g. if bar.h is a
//       necessary #include of foo.h, and foo.cc uses symbols from
//       bar.h, it's optional for foo.cc to #include bar.h.);
//
//     * it's said to be *undesired* if it's an IWYU error to have the
//       #include.
//
//     Therefore, when we say a #include is *desired*, we mean that it's
//     either necessary or optional.
//
//   - We also say that a #include is *recommended* if the IWYU tool
//     recommends to have it in the C++ source file.  Obviously, any
//     necessary #include must be recommended, and no undesired
//     #include can be recommended.  An optional #include can be
//     either recommended or not -- the IWYU tool can decide which
//     case it is.  For example, if foo.cc desires bar.h, but can
//     already get it via foo.h, IWYU won't recommend foo.cc to
//     #include bar.h, unless it already does so.

#if defined(_MSC_VER)
#include <direct.h>
#else
#include <getopt.h>
#include <unistd.h>
#endif
#include <stdio.h>   // for snprintf
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>   // For make_pair
#include <vector>

#include "iwyu_ast_util.h"
#include "iwyu_cache.h"
#include "iwyu_globals.h"
#include "iwyu_include_picker.h"
#include "iwyu_lexer_utils.h"
#include "iwyu_output.h"
#include "iwyu_path_util.h"
#include "iwyu_preprocessor.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Token.h"
#include "clang/Sema/Sema.h"

namespace include_what_you_use {

// I occasionally clean up this list by running:
// $ grep "using clang":: iwyu.cc | cut -b14- | tr -d ";" | while read t; do grep -q "[^:]$t" iwyu.cc || echo $t; done
using clang::ASTConsumer;
using clang::ASTContext;
using clang::ASTFrontendAction;
using clang::CallExpr;
using clang::ClassTemplateDecl;
using clang::ClassTemplateSpecializationDecl;
using clang::CompilerInstance;
using clang::CXXCtorInitializer;
using clang::CXXConstructExpr;
using clang::CXXConstructorDecl;
using clang::CXXDeleteExpr;
using clang::CXXDestructorDecl;
using clang::CXXMethodDecl;
using clang::CXXNewExpr;
using clang::CXXOperatorCallExpr;
using clang::CXXRecordDecl;
using clang::Decl;
using clang::DeclContext;
using clang::DeclRefExpr;
using clang::ElaboratedType;
using clang::Expr;
using clang::FileEntry;
using clang::FriendDecl;
using clang::FriendTemplateDecl;
using clang::FullSourceLoc;
using clang::FunctionDecl;
using clang::FunctionProtoType;
using clang::FunctionTemplateDecl;
using clang::FunctionType;
using clang::ImplicitCastExpr;
using clang::LinkageSpecDecl;
using clang::LValueReferenceType;
using clang::MacroInfo;
using clang::MemberExpr;
using clang::NamedDecl;
using clang::NestedNameSpecifier;
using clang::OverloadExpr;
using clang::PointerType;
using clang::PPCallbacks;
using clang::QualType;
using clang::QualifiedTypeLoc;
using clang::RecordDecl;
using clang::RecordType;
using clang::ReferenceType;
using clang::SizeOfAlignOfExpr;
using clang::SourceLocation;
using clang::SourceManager;
using clang::Stmt;
using clang::SubstTemplateTypeParmType;
using clang::TemplateArgument;
using clang::TemplateArgumentList;
using clang::TemplateArgumentLoc;
using clang::TemplateName;
using clang::TemplateSpecializationKind;
using clang::TemplateSpecializationType;
using clang::TemplateTemplateParmDecl;
using clang::TagDecl;
using clang::TagType;
using clang::TemplateDecl;
using clang::Token;
using clang::Type;
using clang::TypeLoc;
using clang::TypedefDecl;
using clang::TypedefType;
using clang::UsingShadowDecl;
using clang::RecursiveASTVisitor;
using clang::UsingDecl;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::errs;
using llvm::raw_string_ostream;
using std::find;
using std::make_pair;
using std::map;
using std::set;
using std::string;
using std::swap;
using std::vector;

// The default value for the --howtodebug flag.  Indicates that the
// flag isn't present.  It's a special, reserved value, and a user
// isn't expected to type it directly.
const char kFlagUnspecified[] = "<flag-unspecified>";

// Which source file should IWYU print debug instructions for ("" for
// every file being checked)?
static string FLAGS_howtodebug = kFlagUnspecified;  // NOLINT

// What directory was the tool invoked from?
static string FLAGS_cwd;

namespace {

class WarningLessThan {
 public:
  struct Warning {
    Warning(const string& f, int ln, int cn, const string& m, int c)
        : filename(f), line_num(ln), column_num(cn), message(m), count(c) { }
    const string filename;
    const int line_num;
    const int column_num;
    const string message;
    const int count;
  };

  static Warning ParseWarning(const pair<string, int>& warning_and_count) {
    // Lines look like file:lineno:colno: text.
    const vector<string> segs = Split(warning_and_count.first, ":", 4);
    CHECK_(segs.size() == 4);
    return Warning(segs[0], atoi(segs[1].c_str()), atoi(segs[2].c_str()),
                   segs[3], warning_and_count.second);
  }

  bool operator()(const pair<string, int>& a,
                  const pair<string, int>& b) const {
    const Warning& w1 = ParseWarning(a);
    const Warning& w2 = ParseWarning(b);
    if (w1.filename != w2.filename)  return w1.filename < w2.filename;
    if (w1.line_num != w2.line_num)  return w1.line_num < w2.line_num;
    if (w1.column_num != w2.column_num)  return w1.column_num < w2.column_num;
    if (w1.message != w2.message)  return w1.message < w2.message;
    return w1.count < w2.count;
  }
};

string IntToString(int i) {
  char buf[64];   // big enough for any number
  snprintf(buf, sizeof(buf), "%d", i);
  return buf;
}

}  // namespace

// ----------------------------------------------------------------------
// --- BaseAstVisitor
// ----------------------------------------------------------------------
//
// We have a hierarchy of AST visitor classes, to keep the logic
// as clear as possible.  The top level, BaseAstVisitor, has some
// basic, not particularly iwyu-related, functionality:
//
// 1) Maintain current_ast_node_, the current chain in the AST tree.
// 2) Provide functions related to the current location.
// 3) Print each node we're visiting, depending on --verbose settings.
// 4) Add appropriate implicit text.  iwyu acts as if all constructor
//    initializers were explicitly written, all default constructors
//    were explicitly written, etc, even if they're not.  We traverse
//    the implicit stuff as if it were explicit.
// 5) Make sure templates in typedefs are instantiated.  When we see
//    a typedef, we want to simulate creating an instance of the
//    underlying type.  If the underlying type is never actually used,
//    though, and it's a template, clang may never instantiate it.
//    So we have to.
// 6) Add two callbacks that subclasses can override (just like any
//    other AST callback): TraverseImplicitDestructorCall and
//    HandleFunctionCall.  TraverseImplicitDestructorCall is a
//    callback for a "pseudo-AST" node that covers destruction not
//    specified in source, such as a destructor destroying one of the
//    fields in its class.  HandleFunctionCall is a convenience
//    callback that bundles callbacks from many different kinds of
//    function-calling callbacks (CallExpr, CXXConstructExpr, etc)
//    into one place.
//
// To maintain current_ast_node_ properly, this class also implements
// VisitNestedNameSpecifier, VisitTemplateName, VisitTemplateArg, and
// VisitTemplateArgLoc, which are parallel to the Visit*Decl()/etc
// visitors.  Subclasses should override these Visit routines, and not
// the Traverse routine directly.

// Holds all decls (either a class or class template) where we've
// chosen to explicitly instantiate the methods of this decl.  This
// makes sure we don't try to instantiate twice.  Required by (5).
// TODO(csilvers): should clear this when (if) the AST ever changes.
static set<const Decl*> g_explicitly_instantiated_classes;

template <class Derived>
class BaseAstVisitor : public RecursiveASTVisitor<Derived> {
 public:
  typedef RecursiveASTVisitor<Derived> Base;

  // We need to create implicit ctor/dtor nodes, which requires
  // non-const methods on CompilerInstance, so the var can't be const.
  explicit BaseAstVisitor(CompilerInstance* compiler)
      : compiler_(compiler),
        current_ast_node_(NULL) {}

  virtual ~BaseAstVisitor() {}

  //------------------------------------------------------------
  // Pure virtual methods that a subclass must implement.

  // Returns true if we are not interested in the current ast node for
  // any reason (for instance, it lives in a file we're not
  // analyzing).
  virtual bool CanIgnoreCurrentASTNode() const = 0;

  // Returns true if we should print the information for the
  // current AST node, given what file it's in.  For instance,
  // except at very high verbosity levels, we don't print AST
  // nodes from system header files.
  virtual bool ShouldPrintSymbolFromCurrentFile() const = 0;

  // A string to add to the information we print for each symbol.
  // Each subclass can thus annotate if it's handling a node.
  // The return value, if not empty, should start with a space!
  virtual string GetSymbolAnnotation() const = 0;

  //------------------------------------------------------------
  // (1) Maintain current_ast_node_

  // How subclasses can access current_ast_node_;
  const ASTNode* current_ast_node() const { return current_ast_node_; }
  ASTNode* current_ast_node() { return current_ast_node_; }
  void ResetCurrentAstNode() { current_ast_node_ = NULL; }

  bool TraverseDecl(Decl* decl) {
    if (current_ast_node_->StackContainsContent(decl))
      return true;               // avoid recursion
    ASTNode node(decl, *GlobalSourceManager());
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    return Base::TraverseDecl(decl);
  }

  bool TraverseStmt(Stmt* stmt) {
    if (current_ast_node_->StackContainsContent(stmt))
      return true;               // avoid recursion
    ASTNode node(stmt, *GlobalSourceManager());
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    return Base::TraverseStmt(stmt);
  }

  bool TraverseType(QualType qualtype) {
    const Type* type = qualtype.getTypePtr();
    if (current_ast_node_->StackContainsContent(type))
      return true;               // avoid recursion
    ASTNode node(type, *GlobalSourceManager());
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    return Base::TraverseType(qualtype);
  }

  // RecursiveASTVisitor has a hybrid type-visiting system: it will
  // call TraverseTypeLoc when it can, but will call TraverseType
  // otherwise.  For instance, if we see a FunctionDecl, and it
  // exposes the return type via a TypeLoc, it will recurse via
  // TraverseTypeLoc.  If it exposes the return type only via a
  // QualType, though, it will recurse via TraverseType.  The upshot
  // is we need two versions of all the Traverse*Type routines.  (We
  // don't need two versions the Visit*Type routines, since the
  // default behavior of Visit*TypeLoc is to just call Visit*Type.)
  bool TraverseTypeLoc(TypeLoc typeloc) {
    // QualifiedTypeLoc is a bit of a special case in the typeloc
    // system, off to the side.  We don't care about qualifier
    // positions, so avoid the need for special-casing by just
    // traversing the unqualified version instead.
    if (isa<QualifiedTypeLoc>(typeloc)) {
      typeloc = typeloc.getUnqualifiedLoc();
    }
    if (current_ast_node_->StackContainsContent(&typeloc))
      return true;               // avoid recursion
    ASTNode node(&typeloc, *GlobalSourceManager());
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    return Base::TraverseTypeLoc(typeloc);
  }

  bool TraverseNestedNameSpecifier(NestedNameSpecifier* nns) {
    if (nns == NULL)
      return true;
    ASTNode node(nns, *GlobalSourceManager());
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (!this->getDerived().VisitNestedNameSpecifier(nns))
      return false;
    return Base::TraverseNestedNameSpecifier(nns);
  }

  bool TraverseTemplateName(TemplateName template_name) {
    ASTNode node(&template_name, *GlobalSourceManager());
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (!this->getDerived().VisitTemplateName(template_name))
      return false;
    return Base::TraverseTemplateName(template_name);
  }

  bool TraverseTemplateArgument(const TemplateArgument& arg) {
    ASTNode node(&arg, *GlobalSourceManager());
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (!this->getDerived().VisitTemplateArgument(arg))
      return false;
    return Base::TraverseTemplateArgument(arg);
  }

  bool TraverseTemplateArgumentLoc(const TemplateArgumentLoc& argloc) {
    ASTNode node(&argloc, *GlobalSourceManager());
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (!this->getDerived().VisitTemplateArgumentLoc(argloc))
      return false;
    return Base::TraverseTemplateArgumentLoc(argloc);
  }

  //------------------------------------------------------------
  // (2) Provide functions related to the current location.

  SourceLocation CurrentLoc() const {
    CHECK_(current_ast_node_ && "Call CurrentLoc within Visit* or Traverse*");
    const SourceLocation loc = current_ast_node_->GetLocation();
    if (!loc.isValid())
      return loc;
    // If the token is formed via macro concatenation, the spelling
    // location will be in <scratch space>.  Use the instantiation
    // location instead.
    const FullSourceLoc spelling_loc = GetSpellingLoc(loc);
    if (StartsWith(PrintableLoc(spelling_loc), "<scratch "))
      return GetInstantiationLoc(loc);
    else
      return spelling_loc;
  }

  string CurrentFilePath() const {
    return GetFilePath(CurrentLoc());
  }

  const FileEntry* CurrentFileEntry() const {
    return GetFileEntry(CurrentLoc());
  }

  string PrintableCurrentLoc() const {
    return PrintableLoc(CurrentLoc());
  }

  //------------------------------------------------------------
  // (3) Print each node we're visiting.

  // The current file location, the class or decl or type name in
  // brackets, along with annotations such as the indentation depth,
  // etc.
  string AnnotatedName(const string& name) const {
    return (PrintableCurrentLoc() + ": (" +
            IntToString(current_ast_node_->depth()) + GetSymbolAnnotation() +
            (current_ast_node_->in_forward_declare_context() ?
             ", fwd decl" : "") +
            ") [ " + name + " ] ");
  }

  // At verbose level 7 and above, returns a printable version of
  // the pointer, suitable for being emitted after AnnotatedName.
  // At lower verbose levels, returns the empty string.
  string PrintablePtr(const void* ptr) const {
    if (ShouldPrint(7)) {
      char buffer[32];
      snprintf(buffer, sizeof(buffer), "%p ", ptr);
      return buffer;
    }
    return "";
  }

  // The top-level Decl class.  All Decls call this visitor (in
  // addition to any more-specific visitors that apply for a
  // particular decl).
  bool VisitDecl(clang::Decl* decl) {
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName(string(decl->getDeclKindName()) + "Decl")
             << PrintablePtr(decl) << PrintableDecl(decl) << "\n";
    }
    return true;
  }

  bool VisitStmt(clang::Stmt* stmt) {
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName(stmt->getStmtClassName()) << PrintablePtr(stmt);
      PrintStmt(stmt);
      errs() << "\n";
    }
    return true;
  }

  bool VisitType(clang::Type* type) {
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName(string(type->getTypeClassName()) + "Type")
             << PrintablePtr(type) << PrintableType(type) << "\n";
    }
    return true;
  }

  // Make sure our logging message shows we're in the TypeLoc hierarchy.
  bool VisitTypeLoc(clang::TypeLoc typeloc) {
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName(string(typeloc.getTypePtr()->getTypeClassName())
                              + "TypeLoc")
             << PrintableTypeLoc(typeloc) << "\n";
    }
    return true;
  }

  bool VisitNestedNameSpecifier(NestedNameSpecifier* nns) {
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("NestedNameSpecifier")
             << PrintablePtr(nns) << PrintableNestedNameSpecifier(nns) << "\n";
    }
    return true;
  }

  bool VisitTemplateName(TemplateName template_name) {
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("TemplateName")
             << PrintableTemplateName(template_name) << "\n";
    }
    return true;
  }

  bool VisitTemplateArgument(const TemplateArgument& arg) {
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("TemplateArgument")
             << PrintablePtr(&arg) << PrintableTemplateArgument(arg) << "\n";
    }
    return true;
  }

  bool VisitTemplateArgumentLoc(const TemplateArgumentLoc& argloc) {
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("TemplateArgumentLoc")
             << PrintablePtr(&argloc) << PrintableTemplateArgumentLoc(argloc)
             << "\n";
    }
    return true;
  }

  //------------------------------------------------------------
  // (4) Add implicit text.

  // When we see an object that has implicit text that iwyu
  // wants to look at, we make callbacks as if that text had
  // been explicitly written.  Here's text we consider:
  //
  //    * CXXDestructorDecl: a destructor call for each non-POD field
  //      in the dtor's class, and each base type of that class.
  //    * CXXConstructorDecl: a constructor call for each type/base
  //      of the class that is not explicitly listed in an initializer.
  //    * CXXRecordDecl: a CXXConstructorDecl for each implicit
  //      constructor (zero-arg and copy).  A CXXDestructor decl
  //      if the destructor is implicit.  A CXXOperatorCallDecl if
  //      operator= is explicit.

  bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl* decl) {
    if (!Base::TraverseCXXConstructorDecl(decl))  return false;
    if (CanIgnoreCurrentASTNode())  return true;
    // We only care about classes that are actually defined.
    if (!decl || !decl->isThisDeclarationADefinition())  return true;

    // RAV's TraverseCXXConstructorDecl already handles
    // explicitly-written initializers, so we just want the rest.
    for (CXXConstructorDecl::init_const_iterator it = decl->init_begin();
         it != decl->init_end(); ++it) {
      const CXXCtorInitializer* init = *it;
      if (!init->isWritten()) {
        if (!this->getDerived().TraverseStmt(init->getInit()))
          return false;
      }
    }
    return true;
  }

  bool TraverseCXXDestructorDecl(clang::CXXDestructorDecl* decl) {
    if (!Base::TraverseCXXDestructorDecl(decl))  return false;
    if (CanIgnoreCurrentASTNode())  return true;
    // We only care about calls that are actually defined.
    if (!decl || !decl->isThisDeclarationADefinition())  return true;

    // Collect all the fields (and bases) we destroy, and call the dtor.
    set<const Type*> member_types;
    const CXXRecordDecl* record = decl->getParent();
    for (clang::RecordDecl::field_iterator it = record->field_begin();
         it != record->field_end(); ++it) {
      member_types.insert((*it)->getType().getTypePtr());
    }
    for (clang::CXXRecordDecl::base_class_const_iterator
             it = record->bases_begin(); it != record->bases_end(); ++it) {
      member_types.insert(it->getType().getTypePtr());
    }
    for (Each<const Type*> it(&member_types); !it.AtEnd(); ++it) {
      const NamedDecl* member_decl = TypeToDeclAsWritten(*it);
      // We only want those fields that are c++ classes.
      if (const CXXRecordDecl* cxx_field_decl = DynCastFrom(member_decl)) {
        if (const CXXDestructorDecl* field_dtor
            = cxx_field_decl->getDestructor()) {
          if (!this->getDerived().TraverseImplicitDestructorCall(
                  const_cast<CXXDestructorDecl*>(field_dtor), *it))
            return false;
        }
      }
    }
    return true;
  }

  // clang lazily constructs the implicit methods of a C++ class (the
  // default constructor and destructor, etc) -- it only bothers to
  // create a CXXMethodDecl if someone actually calls these classes.
  // But we need to be non-lazy: iwyu depends on analyzing what future
  // code *may* call in a class, not what current code *does*.  So we
  // force all the lazy evaluation to happen here.  This will
  // (possibly) add a bunch of MethodDecls to the AST, as children of
  // decl.  We're hoping it will always be safe to modify the AST
  // while it's being traversed!
  void InstantiateImplicitMethods(CXXRecordDecl* decl) {
    clang::Sema& sema = compiler_->getSema();
    if (!decl->hasDeclaredDefaultConstructor())
      sema.DefineImplicitDefaultConstructor(
          CurrentLoc(), sema.DeclareImplicitDefaultConstructor(decl));
#if 0  // TODO(csilvers): figure out why these crash clang sometimes
    if (!decl->hasDeclaredCopyConstructor())
      sema.DefineImplicitCopyConstructor(
          CurrentLoc(), sema.DeclareImplicitCopyConstructor(decl), 0);
    if (!decl->hasDeclaredCopyAssignment())
      sema.DefineImplicitCopyAssignment(
          CurrentLoc(), sema.DeclareImplicitCopyAssignment(decl));
#endif
    if (!decl->hasDeclaredDestructor())
      sema.DefineImplicitDestructor(
          CurrentLoc(), sema.DeclareImplicitDestructor(decl));
    // clang queues up method instantiations.  We need to process them now.
    sema.PerformPendingInstantiations();
  }

  // clang doesn't bother to set a TypeSourceInfo for implicit
  // methods, since, well, they don't have a location.  But
  // RecursiveASTVisitor crashes without one, so when we lie and say
  // we're not implicit, we have to lie and give a location as well.
  // (We give the null location.)  This is a small memory leak.
  void SetTypeSourceInfoForImplicitMethodIfNeeded(FunctionDecl* decl) {
    if (decl->getTypeSourceInfo() == NULL) {
      ASTContext& ctx = compiler_->getASTContext();
      decl->setTypeSourceInfo(ctx.getTrivialTypeSourceInfo(decl->getType()));
    }
  }

  // RAV.h's TraverseDecl() ignores implicit nodes, so we lie a bit.
  // TODO(csilvers): figure out a more principled way.
  bool TraverseImplicitDeclHelper(clang::FunctionDecl* decl) {
    CHECK_(decl->isImplicit() && "TraverseImplicitDecl is for implicit decls");
    decl->setImplicit(false);
    SetTypeSourceInfoForImplicitMethodIfNeeded(decl);
    bool retval = this->getDerived().TraverseDecl(decl);
    decl->setImplicit(true);
    return retval;
  }

  // Handle implicit methods that otherwise wouldn't be seen by RAV.
  bool TraverseCXXRecordDecl(clang::CXXRecordDecl* decl) {
    if (!Base::TraverseCXXRecordDecl(decl))  return false;
    if (CanIgnoreCurrentASTNode()) return true;
    // We only care about classes that are actually defined.
    if (!decl || !decl->isThisDeclarationADefinition())  return true;

    InstantiateImplicitMethods(decl);

    // Check to see if there are any implicit constructors.  Can be
    // several: implicit default constructor, implicit copy constructor.
    for (CXXRecordDecl::ctor_iterator it = decl->ctor_begin();
         it != decl->ctor_end(); ++it) {
      CXXConstructorDecl* ctor = *it;
      if (ctor->isImplicit()) {
        if (!TraverseImplicitDeclHelper(ctor))
          return false;
      }
    }
    // Check the (single) destructor.
    if (decl->hasDeclaredDestructor() && !decl->hasUserDeclaredDestructor()) {
      if (!TraverseImplicitDeclHelper(decl->getDestructor()))
        return false;
    }
    // There can actually be two operator='s: one const and one not.
    if (decl->hasDeclaredCopyAssignment() &&
        !decl->hasUserDeclaredCopyAssignment()) {
      if (!TraverseImplicitDeclHelper(decl->getCopyAssignmentOperator(true)) ||
          !TraverseImplicitDeclHelper(decl->getCopyAssignmentOperator(false)))
          return false;
    }
    return true;
  }

  //------------------------------------------------------------
  // (5) Make sure templates in typedefs are instantiated.

  // If you do 'typedef MyClass<Foo> Bar', we basically instantiate
  // MyClass<Foo> right there, and report any iwyu violations we see.
  // This is because the typedef causes us to 're-export'
  // MyClass<Foo>: that is, when you write 'typedef Foo Bar', clients
  // can use Bar however they want without having to worry about
  // #including anything except you.  That puts you on the hook for
  // all the #includes that Bar might need, for *anything* one might
  // want to do to a Bar (basically, instantiate it or access methods
  // of it).
  //
  // But if MyClass<Foo> is never actually used in the program, then
  // clang won't bother to create an implicit instantiation of
  // MyClass<Foo>.  To protect against that, we fake an explicit
  // instantiation at the spot of the typedef, by telling clang that
  // in the code 'typedef MyClass<Foo> MyTypedef;', the 'MyClass<Foo>'
  // is actually an explicit instantiation.

  bool TraverseTypedefDecl(clang::TypedefDecl* decl) {
    if (CanIgnoreCurrentASTNode())
      return Base::TraverseTypedefDecl(decl);

    const Type* underlying_type = decl->getUnderlyingType().getTypePtr();
    const Decl* underlying_decl = TypeToDeclAsWritten(underlying_type);
    if (const ClassTemplateSpecializationDecl* specialization_decl =
        DynCastFrom(underlying_decl)) {
      const TemplateSpecializationKind spec_kind
          = specialization_decl->getTemplateSpecializationKind();
      // TSK_Undeclared means that clang didn't see the need to
      // instantiate it.  Note: this call modifies specialization_decl
      // in place.  It does not cause specialization_decl to be
      // inserted into the AST; it's only accessible via this typedef.
      if (spec_kind == clang::TSK_Undeclared) {
        compiler_->getSema().InstantiateClassTemplateSpecialization(
            CurrentLoc(),
            const_cast<ClassTemplateSpecializationDecl*>(specialization_decl),
            clang::TSK_ExplicitInstantiationDefinition,
            false);   // no complaining!
      }
      // If there was already an explicit instantiation (written
      // directly in the code), all members were instantiated then.
      // If not -- either spec_kind was TSK_Undeclared, or was an
      // implicit instantiation -- we need to make sure all members
      // are instantiated.  However, make sure we only ever do it once!
      if (spec_kind != clang::TSK_ExplicitInstantiationDeclaration &&
          spec_kind != clang::TSK_ExplicitInstantiationDefinition &&
          spec_kind != clang::TSK_ExplicitSpecialization &&
          !Contains(g_explicitly_instantiated_classes, specialization_decl)) {
        // TODO(csilvers): this can emit warnings in badinc.cc.  Figure out why.
        compiler_->getSema().InstantiateClassTemplateSpecializationMembers(
            CurrentLoc(),
            const_cast<ClassTemplateSpecializationDecl*>(specialization_decl),
            clang::TSK_ExplicitInstantiationDefinition);
        g_explicitly_instantiated_classes.insert(specialization_decl);
      }
    }
    // We want to do all this instantiation before traversing the
    // typedef decl, since we have some VisitTypedefDecl() calls
    // (below) which look at specialization_decl, and they need to see
    // an instantiated decl.  That's why we do this parent-call last.
    return Base::TraverseTypedefDecl(decl);
  }

  //------------------------------------------------------------
  // (6) Add TraverseImplicitDestructorCall and HandleFunctionCall.

  // TraverseImplicitDestructorCall: This is a callback this class
  // introduces that is a first-class callback just like any AST-node
  // callback.  It is used to cover two places where the compiler
  // destroys objects, but there's no written indication of that in
  // the text: (1) when a local variable or a temporary goes out of
  // scope (NOTE: in this case, we attribute the destruction to the
  // same line as the corresponding construction, not to where the
  // scope ends).  (2) When a destructor destroys one of the fields of
  // a class.  For instance: 'class Foo { MyClass b; }': In addition
  // to executing its body, Foo::~Foo calls MyClass::~Myclass on b.
  // Note we only call this if an actual destructor is being executed:
  // we don't call it when an int goes out of scope!
  //
  // HandleFunctionCall: A convenience callback that 'bundles'
  // the following Expr's, each of which causes one or more
  // function calls when evaluated (though most of them are
  // not a child of CallExpr):
  //    * CallExpr (obviously)
  //    * CXXMemberCallExpr
  //    * CXXOperatorCallExpr -- a call to operatorXX()
  //    * CXXConstructExpr -- calls a constructor to create an object,
  //      and maybe a destructor when the object goes out of scope.
  //    * CXXTemporaryObjectExpr -- subclass of CXXConstructExpr
  //    * CXXNewExpr -- calls operator new and a constructor
  //    * CXXDeleteExpr -- calls operator delete and a destructor
  //    * DeclRefExpr -- if the declref is a function pointer, we
  //      treat it as a function call, since it can act like one
  //      in the future
  //    * ImplicitDestructorCall -- calls a destructor
  // Each of these calls HandleFunctionCall for the function calls
  // it does.  A subclass interested only in function calls, and
  // not exactly what expression caused them, can override
  // HandleFunctionCall.  Note: subclasses should expect that
  // the first argument to HandleFunctionCall may be NULL (e.g. when
  // constructing a built-in type), in which case the handler should
  // immediately return.

  // If the function being called is a member of a class, parent_type
  // is the type of the method's owner (parent), as it is written in
  // the source.  (We need the type-as-written so we can distinguish
  // explicitly-written template args from default template args.)
  bool HandleFunctionCall(clang::FunctionDecl* callee,
                          const clang::Type* parent_type) {
    if (!callee)  return true;
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("FunctionCall")
             << PrintablePtr(callee) << PrintableDecl(callee) << "\n";
    }
    return true;
  }

  bool TraverseImplicitDestructorCall(clang::CXXDestructorDecl* decl,
                                      const Type* type_being_destroyed) {
    if (CanIgnoreCurrentASTNode())  return true;
    if (!decl)  return true;
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("Destruction")
             << PrintableType(type_being_destroyed) << "\n";
    }
    return this->getDerived().HandleFunctionCall(decl, type_being_destroyed);
  }


  bool TraverseCallExpr(clang::CallExpr* expr) {
    if (!Base::TraverseCallExpr(expr))  return false;
    if (CanIgnoreCurrentASTNode())  return true;
    return this->getDerived().HandleFunctionCall(expr->getDirectCallee(),
                                                 TypeOfParentIfMethod(expr));
  }

  bool TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr* expr) {
    if (!Base::TraverseCXXMemberCallExpr(expr))  return false;
    if (CanIgnoreCurrentASTNode())  return true;
    return this->getDerived().HandleFunctionCall(expr->getDirectCallee(),
                                                 TypeOfParentIfMethod(expr));
  }

  bool TraverseCXXOperatorCallExpr(clang::CXXOperatorCallExpr* expr) {
    if (!Base::TraverseCXXOperatorCallExpr(expr))  return false;
    if (CanIgnoreCurrentASTNode())  return true;

    const Type* parent_type = TypeOfParentIfMethod(expr);
    // If we're a free function -- bool operator==(MyClass a, MyClass b) --
    // we still want to have a parent_type, as if we were defined as
    // MyClass::operator==.  So we go through the arguments and take the
    // first one that's a class, and associate the function with that.
    if (!parent_type && GetFirstClassArgument(expr))
      parent_type = GetTypeOf(GetFirstClassArgument(expr));

    return this->getDerived().HandleFunctionCall(expr->getDirectCallee(),
                                                 parent_type);
  }

  bool TraverseCXXConstructExpr(clang::CXXConstructExpr* expr) {
    if (!Base::TraverseCXXConstructExpr(expr))  return false;
    if (CanIgnoreCurrentASTNode())  return true;

    if (!this->getDerived().HandleFunctionCall(expr->getConstructor(),
                                               GetTypeOf(expr)))
      return false;

    // When creating a local variable or a temporary, the constructor
    // is also responsible for destruction (which happens implicitly
    // when the variable goes out of scope).  Only when initializing
    // a field of a class does the constructor not have to worry
    // about destruction.  It turns out it's easier to check for that.
    if (!IsCXXConstructExprInInitializer(current_ast_node())) {
      // Create the destructor if it hasn't been lazily created yet.
      InstantiateImplicitMethods(expr->getConstructor()->getParent());
      if (const CXXDestructorDecl* dtor_decl = GetSiblingDestructorFor(expr)) {
        if (!this->getDerived().TraverseImplicitDestructorCall(
                const_cast<CXXDestructorDecl*>(dtor_decl), GetTypeOf(expr)))
          return false;
      }
    }
    return true;
  }

  bool TraverseCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr* expr) {
    if (!Base::TraverseCXXTemporaryObjectExpr(expr))  return false;
    if (CanIgnoreCurrentASTNode())  return true;

    // In this case, we *know* we're responsible for destruction as well.
    InstantiateImplicitMethods(expr->getConstructor()->getParent());
    CXXConstructorDecl* ctor_decl = expr->getConstructor();
    CXXDestructorDecl* dtor_decl =
        const_cast<CXXDestructorDecl*>(GetSiblingDestructorFor(expr));
    const Type* type = GetTypeOf(expr);
    return (this->getDerived().HandleFunctionCall(ctor_decl, type) &&
            this->getDerived().HandleFunctionCall(dtor_decl, type));
  }

  bool TraverseCXXNewExpr(clang::CXXNewExpr* expr) {
    if (!Base::TraverseCXXNewExpr(expr))  return false;
    if (CanIgnoreCurrentASTNode())  return true;

    const Type* parent_type = expr->getAllocatedType().getTypePtrOrNull();
    // 'new' calls operator new in addition to the ctor of the new-ed type.
    if (FunctionDecl* operator_new = expr->getOperatorNew()) {
      // If operator new is a method, it must (by the semantics of
      // per-class operator new) be a method on the class we're newing.
      const Type* op_parent = NULL;
      if (isa<CXXMethodDecl>(operator_new))
        op_parent = parent_type;
      if (!this->getDerived().HandleFunctionCall(operator_new, op_parent))
        return false;
    }
    return this->getDerived().HandleFunctionCall(
        expr->getConstructor(), parent_type);
  }

  bool TraverseCXXDeleteExpr(clang::CXXDeleteExpr* expr) {
    if (!Base::TraverseCXXDeleteExpr(expr))  return false;

    if (CanIgnoreCurrentASTNode())  return true;

    const Type* parent_type = expr->getDestroyedType().getTypePtrOrNull();
    // We call operator delete in addition to the dtor of the deleted type.
    if (FunctionDecl* operator_delete = expr->getOperatorDelete()) {
      // If operator delete is a method, it must (by the semantics of per-
      // class operator delete) be a method on the class we're deleting.
      const Type* op_parent = NULL;
      if (isa<CXXMethodDecl>(operator_delete))
        op_parent = parent_type;
      if (!this->getDerived().HandleFunctionCall(operator_delete, op_parent))
        return false;
    }
    const CXXDestructorDecl* dtor = GetDestructorForDeleteExpr(expr);
    return this->getDerived().HandleFunctionCall(
        const_cast<CXXDestructorDecl*>(dtor), parent_type);
  }

  // This is to catch assigning template functions to function pointers.
  // For instance, 'MyFunctionPtr p = &TplFn<MyClass*>;': we need to
  // expand TplFn to see if it needs full type info for MyClass.
  bool TraverseDeclRefExpr(clang::DeclRefExpr* expr) {
    if (!Base::TraverseDeclRefExpr(expr))  return false;
    if (CanIgnoreCurrentASTNode())  return true;

    // If it's a normal function call, that was already handled by a
    // CallExpr somewhere.  We want only assignments.
    if (current_ast_node()->template ParentIsA<CallExpr>() ||
        (current_ast_node()->template ParentIsA<ImplicitCastExpr>() &&
         current_ast_node()->template AncestorIsA<CallExpr>(2))) {
      return true;
    }

    if (FunctionDecl* fn_decl = DynCastFrom(expr->getDecl())) {
      // If fn_decl has a class-name before it -- 'MyClass::method' --
      // it's a method pointer.
      const Type* parent_type = NULL;
      if (expr->getQualifier() && expr->getQualifier()->getAsType())
        parent_type = expr->getQualifier()->getAsType();
      if (!this->getDerived().HandleFunctionCall(fn_decl, parent_type))
        return false;
    }
    return true;
  }

 protected:
  CompilerInstance* compiler() { return compiler_; }

 private:
  template <typename T> friend class BaseAstVisitor;
  CompilerInstance* const compiler_;

  // The currently active decl/stmt/type/etc -- that is, the node
  // being currently visited in a Visit*() or Traverse*() method.  The
  // advantage of ASTNode over the object passed in to Visit*() and
  // Traverse*() is ASTNode knows its parent.
  ASTNode* current_ast_node_;
};


// ----------------------------------------------------------------------
// --- AstTreeFlattenerVisitor
// ----------------------------------------------------------------------
//
// This simple visitor just creates a set of all AST nodes (stored as
// void*'s) seen while traversing via BaseAstVisitor.

class AstFlattenerVisitor : public BaseAstVisitor<AstFlattenerVisitor> {
 public:
  typedef BaseAstVisitor<AstFlattenerVisitor> Base;

  // We divide our set of nodes into category by type.  For most AST
  // nodes, we can store just a pointer to the node.  However, for
  // some AST nodes we don't get a pointer into the AST, we get a
  // temporary (stack-allocated) object, and have to store the full
  // object ourselves and use its operator== to test for equality.
  // These types each get their own set (or, usually, vector, since
  // the objects tend not to support operator< or hash<>()).
  class NodeSet {
   public:
    // We could add more versions, but these are the only useful ones so far.
    bool Contains(const Type* type) const {
      return include_what_you_use::Contains(others, type);
    }
    bool Contains(const Decl* decl) const {
      return include_what_you_use::Contains(others, decl);
    }
    bool Contains(const ASTNode& node) const {
      if (const TypeLoc* tl = node.GetAs<TypeLoc>()) {
        return find(typelocs.begin(), typelocs.end(), *tl) != typelocs.end();
      } else if (const TemplateName* tn = node.GetAs<TemplateName>()) {
        // The best we can do is to compare the associated decl
        if (tn->getAsTemplateDecl() == NULL)
          return false;    // be conservative if we can't compare decls
        for (Each<TemplateName> it(&tpl_names); it.AtEnd(); ++it) {
          if (it->getAsTemplateDecl() == tn->getAsTemplateDecl())
            return true;
        }
        return false;
      } else if (const TemplateArgument* ta = node.GetAs<TemplateArgument>()) {
        // TODO(csilvers): figure out how to compare template arguments
        (void)ta;
        return false;
      } else if (const TemplateArgumentLoc* tal =
                 node.GetAs<TemplateArgumentLoc>()) {
        // TODO(csilvers): figure out how to compare template argument-locs
        (void)tal;
        return false;
      } else {
        return others.find(node.GetAs<void>()) != others.end();
      }
    }

    void AddAll(const NodeSet& that) {
      Extend(&typelocs, that.typelocs);
      Extend(&tpl_names, that.tpl_names);
      Extend(&tpl_args, that.tpl_args);
      Extend(&tpl_arglocs, that.tpl_arglocs);
      InsertAllInto(that.others, &others);
    }

    // Needed since we're treated like an stl-like object.
    bool empty() const {
      return (typelocs.empty() && tpl_names.empty() && tpl_args.empty() &&
              tpl_arglocs.empty() && others.empty());
    }
    void clear() {
      typelocs.clear();
      tpl_names.clear();
      tpl_args.clear();
      tpl_arglocs.clear();
      others.clear();
    }

   private:
    friend class AstFlattenerVisitor;

    // It's ok not to check for duplicates; we're just traversing the tree.
    void Add(TypeLoc tl) { typelocs.push_back(tl); }
    void Add(TemplateName tn) { tpl_names.push_back(tn); }
    void Add(TemplateArgument ta) { tpl_args.push_back(ta); }
    void Add(TemplateArgumentLoc tal) { tpl_arglocs.push_back(tal); }
    void Add(const void* o) { others.insert(o); }

    vector<TypeLoc> typelocs;
    vector<TemplateName> tpl_names;
    vector<TemplateArgument> tpl_args;
    vector<TemplateArgumentLoc> tpl_arglocs;
    set<const void*> others;
  };

  //------------------------------------------------------------
  // Public interface:

  explicit AstFlattenerVisitor(CompilerInstance* compiler) : Base(compiler) { }

  const NodeSet& GetNodesBelow(Decl* decl) {
    CHECK_(seen_nodes_.empty() && "Nodes should be clear before GetNodesBelow");
    NodeSet* node_set = &nodeset_decl_cache_[decl];
    if (node_set->empty()) {
      if (decl->isImplicit()) {
        TraverseImplicitDeclHelper(dyn_cast_or_null<FunctionDecl>(decl));
      } else {
        TraverseDecl(decl);
      }
      swap(*node_set, seen_nodes_);  // move the seen_nodes_ into the cache
    }
    return *node_set;                // returns the cache entry
  }

  //------------------------------------------------------------
  // Pure virtual methods that the base class requires.

  virtual bool CanIgnoreCurrentASTNode() const {
    return false;
  }
  virtual bool ShouldPrintSymbolFromCurrentFile() const {
    return false;
  }
  virtual string GetSymbolAnnotation() const {
    return "[Uninstantiated template AST-node] ";
  }

  //------------------------------------------------------------
  // Top-level handlers that construct the tree.

  bool VisitDecl(Decl*) { AddCurrentAstNodeAsPointer(); return true; }
  bool VisitStmt(Stmt*) { AddCurrentAstNodeAsPointer(); return true; }
  bool VisitType(Type*) { AddCurrentAstNodeAsPointer(); return true; }
  bool VisitTypeLoc(TypeLoc typeloc) {
    VERRS(7) << GetSymbolAnnotation() << PrintableTypeLoc(typeloc) << "\n";
    seen_nodes_.Add(typeloc);
    return true;
  }
  bool VisitNestedNameSpecifier(NestedNameSpecifier*) {
    AddCurrentAstNodeAsPointer();
    return true;
  }
  bool VisitTemplateName(TemplateName tpl_name) {
    VERRS(7) << GetSymbolAnnotation()
             << PrintableTemplateName(tpl_name) << "\n";
    seen_nodes_.Add(tpl_name);
    return true;
  }
  bool VisitTemplateArgument(const TemplateArgument& tpl_arg) {
    VERRS(7) << GetSymbolAnnotation()
             << PrintableTemplateArgument(tpl_arg) << "\n";
    seen_nodes_.Add(tpl_arg);
    return true;
  }
  bool VisitTemplateArgumentLoc(const TemplateArgumentLoc& tpl_argloc) {
    VERRS(7) << GetSymbolAnnotation()
             << PrintableTemplateArgumentLoc(tpl_argloc) << "\n";
    seen_nodes_.Add(tpl_argloc);
    return true;
  }
  bool TraverseImplicitDestructorCall(clang::CXXDestructorDecl* decl,
                                      const Type* type) {
    VERRS(7) << GetSymbolAnnotation() << "[implicit dtor] "
             << static_cast<void*>(decl)
             << (decl ? PrintableDecl(decl) : "NULL") << "\n";
    AddAstNodeAsPointer(decl);
    return Base::TraverseImplicitDestructorCall(decl, type);
  }
  bool HandleFunctionCall(clang::FunctionDecl* callee,
                          const clang::Type* parent_type) {
    VERRS(7) << GetSymbolAnnotation() << "[function call] "
             << static_cast<void*>(callee)
             << (callee ? PrintableDecl(callee) : "NULL") << "\n";
    AddAstNodeAsPointer(callee);
    return Base::HandleFunctionCall(callee, parent_type);
  }

  //------------------------------------------------------------
  // Class logic.

  void AddAstNodeAsPointer(const void* node) {
    seen_nodes_.Add(node);
  }

  void AddCurrentAstNodeAsPointer() {
    if (ShouldPrint(7)) {
      errs() << GetSymbolAnnotation() << current_ast_node()->GetAs<void>()
             << " ";
      PrintASTNode(current_ast_node());
      errs() << "\n";
    }
    AddAstNodeAsPointer(current_ast_node()->GetAs<void>());
  }

 private:
  NodeSet seen_nodes_;

  // Because we make a new AstFlattenerVisitor each time we flatten, we
  // need to make this map static.
  // TODO(csilvers): just have one flattener, so this needn't be static.
  static map<const Decl*, NodeSet> nodeset_decl_cache_;
};

map<const Decl*, AstFlattenerVisitor::NodeSet>
AstFlattenerVisitor::nodeset_decl_cache_;


// ----------------------------------------------------------------------
// --- IwyuBaseAstVisitor
// ----------------------------------------------------------------------
//
// We use two AST visitor classes to implement IWYU: IwyuAstConsumer
// is the main visitor that traverses the AST corresponding to what's
// actually written in the source code, and
// InstantiatedTemplateVisitor is for traversing template
// instantiations.  This class template holds iwyu work that is be
// shared by both.

template <class Derived>
class IwyuBaseAstVisitor : public BaseAstVisitor<Derived> {
 public:
  typedef BaseAstVisitor<Derived> Base;

  IwyuBaseAstVisitor(CompilerInstance* compiler,
                     const IwyuPreprocessorInfo& preprocessor_info)
      : Base(compiler),
        preprocessor_info_(preprocessor_info),
        processed_overload_locs_() {}

  virtual ~IwyuBaseAstVisitor() {}

  // To avoid having this-> pointers everywhere, we re-export Base's
  // functions that we use in this class.  This is a language nit(?)
  // when a templated class subclasses from another templated class.
  using Base::CanIgnoreCurrentASTNode;
  using Base::CurrentLoc;
  using Base::CurrentFileEntry;
  using Base::PrintableCurrentLoc;
  using Base::current_ast_node;

  //------------------------------------------------------------
  // Pure virtual methods that a subclass must implement.

  // Returns true if we are not interested in iwyu information for the
  // given type, where the type is *not* the current AST node.
  // TODO(csilvers): check we're calling this consistent with its API.
  virtual bool CanIgnoreType(const Type* type) const = 0;

  // Returns true if we are not interested in doing an iwyu check on
  // the given decl, where the decl is *not* the current AST node.
  // TODO(csilvers): check we're calling this consistent with its API.
  virtual bool CanIgnoreDecl(const Decl* decl) const = 0;

  //------------------------------------------------------------
  // IWYU logic.

  // Helper for MapPrivateDeclToPublicDecl.  Returns true if the decl
  // is a template specialization whose (fully qualified) name matches
  // the given name, has the given number of template arguments, and
  // whose specified tpl argument is a type.
  bool DeclIsTemplateWithNameAndNumArgsAndTypeArg(
      const Decl* decl, const string& name,
      size_t num_args, size_t type_arg_idx) const {
    const ClassTemplateSpecializationDecl* tpl_decl = DynCastFrom(decl);
    if (!tpl_decl)
      return false;
    const string actual_name = tpl_decl->getQualifiedNameAsString();
    if (name != actual_name)
      return false;
    const TemplateArgumentList& tpl_args = tpl_decl->getTemplateArgs();
    if (tpl_args.size() != num_args)
      return false;
    if (tpl_args.get(type_arg_idx).getKind() != TemplateArgument::Type)
      return false;
    return true;
  }

  // This requires the above function to have been called on decl, first.
  const Type* GetTplTypeArg(const Decl* decl, size_t type_arg_idx) const {
    const ClassTemplateSpecializationDecl* tpl_decl = DynCastFrom(decl);
    CHECK_(tpl_decl && "Must call DeclIsTemplateWithNameAndNumArgsAndTypeArg");
    const TemplateArgumentList& tpl_args = tpl_decl->getTemplateArgs();
    CHECK_(tpl_args.size() > type_arg_idx && "Invalid type_arg_idx");
    CHECK_(tpl_args.get(type_arg_idx).getKind() == TemplateArgument::Type);
    return tpl_args.get(type_arg_idx).getAsType().getTypePtr();
  }

  // Some types, such as __gnu_cxx::__normal_iterator, are private
  // types that should not be exposed to the user.  Instead, they're
  // exposed to the user via typedefs, like vector::iterator.
  // Sometimes, the typedef gets lost (such as for find(myvec.begin(),
  // myvec.end(), foo)), so we need to manually map back.  We map
  // __normal_iterator<foo, vector> to vector<>, assuming that the
  // vector<> class includes the typedef.  Likewise, we map any free
  // function taking a __normal_iterator<foo, vector> (such as
  // operator==) to vector<>, assuming that that (templatized)
  // function is instantiated as part of the vector class.  If the
  // input decl does not correspond to a private decl, we return NULL.
  // This method is actually a helper for MapPrivateDeclToPublicDecl()
  // and MapPrivateTypeToPublicType().
  const Type* MapPrivateDeclToPublicType(const NamedDecl* decl) const {
    const NamedDecl* class_decl = decl;
    // If we're a member method, then the __normal_iterator will be
    // the parent: __normal_iterator::operator=.  If we're a free
    // overloaded operator, then the __normal_iterator will be the
    // first argument: operator==(__normal_iterator<...>& lhs, ...);
    if (const CXXMethodDecl* method_decl = DynCastFrom(class_decl)) {
      class_decl = method_decl->getParent();
    } else if (const FunctionDecl* fn = DynCastFrom(decl)) {
      if (fn->isOverloadedOperator() && fn->getNumParams() >= 1) {
        const Type* firstarg_type = fn->getParamDecl(0)->getType().getTypePtr();
        firstarg_type = RemovePointersAndReferencesAsWritten(firstarg_type);
        class_decl = TypeToDeclAsWritten(firstarg_type);
      }
    }

    // In addition to __normal_iterator<x>, we want to handle
    // reverse_iterator<__normal_iterator<x>>, and in the same way.
    if (DeclIsTemplateWithNameAndNumArgsAndTypeArg(
            class_decl, "std::reverse_iterator", 1, 0)) {
      const Type* ni_type = GetTplTypeArg(class_decl, 0);
      // Gets class_decl to be '__normal_iterator<x>'.
      class_decl = TypeToDeclAsWritten(ni_type);
    }

    if (DeclIsTemplateWithNameAndNumArgsAndTypeArg(
            class_decl, "__gnu_cxx::__normal_iterator", 2, 1)) {
      return GetTplTypeArg(class_decl, 1);
    }
    return NULL;
  }

  const NamedDecl* MapPrivateDeclToPublicDecl(const NamedDecl* decl) const {
    const Type* public_type = MapPrivateDeclToPublicType(decl);
    if (public_type)
      return TypeToDeclAsWritten(public_type);
    return decl;
  }

  const Type* MapPrivateTypeToPublicType(const Type* type) const {
    const NamedDecl* private_decl = TypeToDeclAsWritten(type);
    const Type* public_type = MapPrivateDeclToPublicType(private_decl);
    if (public_type)
      return public_type;
    return type;
  }

  //------------------------------------------------------------
  // Checkers, that tell iwyu_output about uses of symbols.
  // We let, but don't require, subclasses to override these.

  // Called when the given type is fully used at used_loc, regardless
  // of the type being explicitly written in the source code or not.
  virtual void ReportTypeUse(SourceLocation used_loc, const Type* type) {
    // TODO(csilvers): figure out if/when calling CanIgnoreType() is correct.

    // Map private types like __normal_iterator to their public counterpart.
    type = MapPrivateTypeToPublicType(type);
    const FileEntry* used_in = GetFileEntry(used_loc);
    preprocessor_info().FileInfoFor(used_in)->ReportFullSymbolUse(
        used_loc, type, IsNodeInsideCXXMethodBody(current_ast_node()));
  }

  virtual void ReportTypesUse(SourceLocation used_loc,
                              const set<const Type*>& types) {
    for (Each<const Type*> it(&types); !it.AtEnd(); ++it)
      ReportTypeUse(used_loc, *it);
  }

  // Called when a type is used in a forward-declare context.
  virtual void ReportTypeForwardDeclareUse(SourceLocation used_loc,
                                           const Type* type) {
    // TODO(csilvers): figure out if/when calling CanIgnoreType() is correct.
    type = MapPrivateTypeToPublicType(type);
    const FileEntry* used_in = GetFileEntry(used_loc);
    preprocessor_info().FileInfoFor(used_in)->ReportForwardDeclareUse(
        used_loc, type, IsNodeInsideCXXMethodBody(current_ast_node()));
  }

  virtual void ReportDeclUse(SourceLocation used_loc, const NamedDecl* decl) {
    // Map private decls like __normal_iterator to their public counterpart.
    decl = MapPrivateDeclToPublicDecl(decl);
    if (CanIgnoreDecl(decl))
      return;
    const FileEntry* used_in = GetFileEntry(used_loc);
    preprocessor_info().FileInfoFor(used_in)->ReportFullSymbolUse(
        used_loc, decl, IsNodeInsideCXXMethodBody(current_ast_node()));
  }

  virtual void ReportDeclsUse(SourceLocation used_loc,
                              const set<const NamedDecl*>& decls) {
    for (Each<const NamedDecl*> it(&decls); !it.AtEnd(); ++it)
      ReportDeclUse(used_loc, *it);
  }

  virtual void ReportDeclForwardDeclareUse(SourceLocation used_loc,
                                           const NamedDecl* decl) {
    decl = MapPrivateDeclToPublicDecl(decl);
    if (CanIgnoreDecl(decl))
      return;
    const FileEntry* used_in = GetFileEntry(used_loc);
    preprocessor_info().FileInfoFor(used_in)->ReportForwardDeclareUse(
        used_loc, decl, IsNodeInsideCXXMethodBody(current_ast_node()));
  }

  //------------------------------------------------------------
  // Visitors of types derived from clang::Decl.

  // Friend declarations only need their types forward-declared.
  bool VisitFriendDecl(clang::FriendDecl* decl) {
    if (CanIgnoreCurrentASTNode())  return true;
    current_ast_node()->set_in_forward_declare_context(true);
    return true;
  }

  bool VisitFriendTemplateDecl(clang::FriendTemplateDecl* decl) {
    if (CanIgnoreCurrentASTNode())  return true;
    current_ast_node()->set_in_forward_declare_context(true);
    return true;
  }

  // If you say 'typedef Foo Bar', we want to decide if you are
  // 're-export' type Foo under the name Bar.  If so, iwyu would
  // require you to fully define Foo (even though C++ doesn't).
  // Currently, we say most all typedefs are re-exporting.  However,
  // we make exceptions for situations we know the typedef is not
  // actually re-exporting anything useful:
  // 1) There's no definition for the class (weird, but legal):
  //       class NeverDefined; typedef NeverDefined Foo;
  //    In that case, we *can't* require the full definition,
  //    because it doesn't exist.
  // 2) The typedef is a member of a templated class, and the
  //    underlying type is a template parameter:
  //       template<class T> struct C { typedef T value_type; };
  //    This is not a re-export because you need the type to
  //    access the typedef (via 'C<Someclass>::value_type'), so
  //    there's no need for the typedef-file to provide the type
  //    too.
  // As an extension of (2), if the typedef is a template type that
  // contains T as a template parameter, the typedef still re-exports
  // the template type (it's not (2)), but the template parameter
  // itself can be forward-declared, just as in (2).  That is:
  //   template<class T> struct C { typedef pair<T,T> value_type; };
  // iwyu will demand the full type of pair, but not of its template
  // arguments.  This is handled not here, but below, in
  // DetermineForwardDeclareStatusForTemplateArg.
  //
  // There may be other exceptions we could add, but these are the
  // important ones.  (2) is important to avoid requiring the full
  // type info for scoped_ptr<MyClass> -- by design, scoped_ptr
  // should only need the full type info of its arg for its destructor.
  bool TypedefIsReExportingUnderlyingType(const clang::TypedefDecl* decl,
                                          const ASTNode* typedef_ast_node) {
    const Type* underlying_type = decl->getUnderlyingType().getTypePtr();
    const NamedDecl* underlying_decl = TypeToDeclAsWritten(underlying_type);

    if (underlying_decl && GetDefinitionForClass(underlying_decl) == NULL)
      return false;    // case (1)

    if (isa<SubstTemplateTypeParmType>(underlying_type))
      return false;  // case (2)

    return true;
  }

  // If you say 'typedef Foo Bar', the language says you just need to
  // forward-declare Foo.  But we may require you to fully define it
  // if we think the typedef means you're 're-exporting' the type
  // under a new name.
  bool VisitTypedefDecl(clang::TypedefDecl* decl) {
    if (CanIgnoreCurrentASTNode())  return true;
    current_ast_node()->set_in_forward_declare_context(
        !TypedefIsReExportingUnderlyingType(decl, current_ast_node()));
    return Base::VisitTypedefDecl(decl);
  }

  // If we're a declared (not defined) function, all our types --
  // return type and argument types -- are forward-declarable.  The
  // one exception is the throw types, which we clean up in
  // VisitType().
  // Also, if any of our function parameters have a type with a
  // non-explicit, one-arg constructor, or is a const reference to
  // such a type, mark that type as not forward declarable.  The
  // worry is that people might need the full type for the implicit
  // conversion, for instance, passing in a char* to
  // Fn(const StringPiece& foo) { ... }
  bool VisitFunctionDecl(clang::FunctionDecl* decl) {
    if (CanIgnoreCurrentASTNode())  return true;

    if (!decl->isThisDeclarationADefinition())
      // Make all our types forward-declarable.
      current_ast_node()->set_in_forward_declare_context(true);

    // Check for the non-explicit, one-arg constructor types.
    for (FunctionDecl::param_iterator param = decl->param_begin();
         param != decl->param_end(); ++param) {
      const Type* param_type = GetTypeOf(*param);
      if (!CanImplicitlyConvertTo(param_type))
        continue;
      const Type* deref_param_type =
          RemovePointersAndReferencesAsWritten(param_type);
      if (CanIgnoreType(param_type) && CanIgnoreType(deref_param_type))
        continue;

      // TODO(csilvers): remove this 'if' check when we've resolved the
      // clang bug where getTypeSourceInfo() can return NULL.
      if ((*param)->getTypeSourceInfo()) {
        const TypeLoc param_tl = (*param)->getTypeSourceInfo()->getTypeLoc();
        // This is a 'full type required' check, to 'turn off' fwd decl.
        ReportTypeUse(GetLocation(&param_tl), deref_param_type);
      } else {
        VERRS(6) << "WARNING: NULL TypeSourceInfo for " << PrintableDecl(*param)
                 << " (type " << PrintableType(param_type) << ")\n";
      }
    }
    return true;
  }

  //------------------------------------------------------------
  // Visitors of types derived from clang::Stmt.

  // When casting non-pointers, iwyu will do the right thing
  // automatically, but sometimes when casting from one pointer to
  // another, you still need the full type information of both types:
  // for instance, when static-casting from a sub-class to a
  // super-class.  Testing shows this is true for static, dynamic
  // casts, and implicit casts, but not for reinterpret casts, const
  // casts, or C-style casts.  (Functional casts like int(3.5) are
  // treated the same as C-style casts.)  clang helpfully provides a
  // 'cast kind', which we use to determine when full types are
  // needed.  When we notice that the cast is a cast up or down a
  // class hierarchy, we require full type info for both types even
  // for C-style casts (though the language doesn't), to give the
  // compiler a fighting chance of generating correct code.
  bool VisitCastExpr(clang::CastExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;
    const Type* from_type = GetTypeOf(expr->getSubExprAsWritten());
    const Type* to_type = GetTypeOf(expr);

    // For all those casts that don't result in function calls
    // (everything except a user-defined cast or a constructor cast),
    // we only care about the need for full types when casting either
    // a pointer to a pointer, or any type to a reference.
    // Unfortunately, when casting to a reference, clang seems to
    // strip the reference off of to_type, so we need a separate
    // function call to tell.
    if (expr->getCastKind() != clang::CK_UserDefinedConversion &&
        expr->getCastKind() != clang::CK_ConstructorConversion) {
      if (!((from_type->hasPointerRepresentation() &&   // pointer or reference
             to_type->hasPointerRepresentation()) ||
            IsCastToReferenceType(expr)))
        return true;    // we only care about ptr-to-ptr casts for this check
    }

    bool need_full_from_type = false;
    bool need_full_to_type = false;
    // The list of kinds: http://clang.llvm.org/doxygen/namespaceclang.html
    switch (expr->getCastKind()) {
      // This cast still isn't handled directly.
      case clang::CK_Dependent:
        break;

      // These casts don't require any iwyu action.
      case clang::CK_LValueToRValue:
        break;

      // We shouldn't be seeing any of these kinds.
      case clang::CK_ArrayToPointerDecay:
      case clang::CK_FloatingCast:
      case clang::CK_FloatingComplexCast:
      case clang::CK_FloatingComplexToBoolean:
      case clang::CK_FloatingComplexToIntegralComplex:
      case clang::CK_FloatingComplexToReal:
      case clang::CK_FloatingRealToComplex:
      case clang::CK_FloatingToBoolean:
      case clang::CK_FloatingToIntegral:
      case clang::CK_FunctionToPointerDecay:
      case clang::CK_IntegralCast:
      case clang::CK_IntegralComplexCast:
      case clang::CK_IntegralComplexToBoolean:
      case clang::CK_IntegralComplexToFloatingComplex:
      case clang::CK_IntegralComplexToReal:
      case clang::CK_IntegralRealToComplex:
      case clang::CK_IntegralToBoolean:
      case clang::CK_IntegralToFloating:
      case clang::CK_IntegralToPointer:
      case clang::CK_MemberPointerToBoolean:
      case clang::CK_NullToMemberPointer:
      case clang::CK_NullToPointer:
      case clang::CK_PointerToBoolean:
      case clang::CK_PointerToIntegral:
      case clang::CK_ToUnion:
      case clang::CK_ToVoid:
        // Due to a bug in clang, we sometimes get IntegralToPointer
        // kinds for a cast that should be a NoOp kind:
        //    http://llvm.org/bugs/show_bug.cgi?id=8543
        // It's possible clang mis-categorizes in other cases too.  So
        // I just log here, rather than asserting and possibly
        // crashing iwyu.
        VERRS(3) << "WARNING: Unexpected cast that involves a non-pointer: "
                 << expr->getCastKindName() << "\n";
        break;
      case clang::CK_AnyPointerToObjCPointerCast:
      case clang::CK_AnyPointerToBlockPointerCast:
      case clang::CK_GetObjCProperty:
      case clang::CK_ObjCObjectLValueCast:
      case clang::CK_VectorSplat:
        CHECK_(false && "TODO(csilvers): for objc and clang lang extensions");
        break;

      // Kinds for reinterpret_cast and const_cast, which need no full types.
      case clang::CK_BitCast:                // used for reinterpret_cast
      case clang::CK_LValueBitCast:          // used for reinterpret_cast
      case clang::CK_NoOp:                   // used for const_cast, etc
        break;

      // Need the full to-type so we can call its constructor.
      case clang::CK_ConstructorConversion:
        need_full_to_type = true;
        break;
      // Need the full from-type so we can call its 'operator <totype>()'.
      case clang::CK_UserDefinedConversion:
        need_full_from_type = true;
        break;

      // Kinds that cast up or down an inheritance hierarchy.
      case clang::CK_BaseToDerived:
      case clang::CK_BaseToDerivedMemberPointer:
        need_full_to_type = true;  // full type for derived gets base type too
        break;
      case clang::CK_DerivedToBase:
      case clang::CK_UncheckedDerivedToBase:
      case clang::CK_DerivedToBaseMemberPointer:
        need_full_from_type = true;
        break;
      case clang::CK_Dynamic:
        // Usually dynamic casting is a base-to-derived cast, but it is
        // possible to dynamic-cast between siblings, in which case we
        // need both types.
        need_full_from_type = true;
        need_full_to_type = true;
        break;
    }

    if (need_full_from_type && !CanIgnoreType(from_type)) {
      ReportTypeUse(CurrentLoc(), RemovePointersAndReferences(from_type));
    }
    if (need_full_to_type && !CanIgnoreType(to_type)) {
      ReportTypeUse(CurrentLoc(), RemovePointersAndReferences(to_type));
    }
    return true;
  }

  // Mark that we need the full type info for our base type -- the
  // thing we're a member of -- and it's not just forward-declarable.
  // For instance, for code 'Mytype* myvar; myvar->a;', we'll get a
  // MemberExpr callback whose base has the type of myvar.
  bool VisitMemberExpr(clang::MemberExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;

    const Expr* base_expr = expr->getBase()->IgnoreParenImpCasts();
    const Type* base_type = GetTypeOf(base_expr);
    CHECK_(base_type && "Member's base does not have a type?");
    const Type* deref_base_type      // For myvar->a, base-type will have a *
        = expr->isArrow() ? RemovePointerFromType(base_type) : base_type;
    if (CanIgnoreType(base_type) && CanIgnoreType(deref_base_type))
      return true;
    // Technically, we should say the type is being used at the
    // location of base_expr.  That may be a different file than us in
    // cases like MACRO.b().  However, while one can imagine
    // situations where the base-type is the responsibility of the
    // macro-author ('SOME_GLOBAL_OBJECT.a()'), the more common case
    // is it's our responsibility ('CHECK_NOTNULL(x).a()').  Until we
    // can better distinguish whether a macro body is an expression
    // that's responsible for its type or not, we just assume it is.
    // TODO(csilvers): fix when we can determine what the macro-text
    // is responsible for and what we're responsible for.
    ReportTypeUse(CurrentLoc(), deref_base_type);
    return true;
  }

  // Mark that we need the full type info for the thing we're taking
  // sizeof of.  Sometimes this is double-counting: for
  // sizeof(some_type), RecursiveASTVisitor will visit some_type and
  // say it needs the full type information there, and for
  // sizeof(some_var), we'll report we need full type information when
  // some_var is defined.  But if the arg is a reference, nobody else
  // will say we need full type info but us.
  bool VisitSizeOfAlignOfExpr(clang::SizeOfAlignOfExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;

    // Calling sizeof on a reference-to-X is the same as calling it on X.
    // If sizeof() takes a type, this is easy to check.  If sizeof()
    // takes an expr, it's hard to tell -- GetTypeOf(expr) 'sees through'
    // references.  Luckily, we want to see through references, so we
    // just use the GetTypeOf().
    if (expr->isArgumentType()) {
      const TypeLoc& arg_tl = expr->getArgumentTypeInfo()->getTypeLoc();
      if (const ReferenceType* reftype = DynCastFrom(arg_tl.getTypePtr())) {
        const Type* dereftype = reftype->getPointeeTypeAsWritten().getTypePtr();
        if (!CanIgnoreType(reftype) || !CanIgnoreType(dereftype))
          ReportTypeUse(GetLocation(&arg_tl), dereftype);
      } else {
        // No need to report on non-ref types, RecursiveASTVisitor will get 'em.
      }
    } else {
      const Expr* arg_expr = expr->getArgumentExpr();
      const Type* dereftype = arg_expr->getType().getTypePtr();
      if (!CanIgnoreType(dereftype))
        // This reports even if the expr ends up not being a reference, but
        // that's ok (if potentially redundant).
        ReportTypeUse(GetLocation(arg_expr), dereftype);
    }
    return true;
  }

  // We want to mark use of the base type For 'free function' operator
  // overloads ('ostream& operator<<(ostream& o, int x)') just like we
  // do for member functions ('ostream& ostream::operator<<(int x)')
  // -- for iwyu purposes, 'x << 4' is just semantic sugar around
  // x.operator<<(4).
  bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;

    if (const Expr* owner_expr = GetFirstClassArgument(expr)) {
      const Type* owner_type = GetTypeOf(owner_expr);
      // Note we report the type use is the location of owner_expr
      // (the 'a' in 'a << b' or the 'MACRO' in 'MACRO << b'), rather
      // than our location (which is the '<<').  That way, we properly
      // situate the owner when it's a macro.
      if (!CanIgnoreType(owner_type))
        ReportTypeUse(GetLocation(owner_expr), owner_type);
    }
    return true;
  }

  // We have to check the type being deleted is fully defined (the
  // language doesn't require it, but bad things happen if it's not:
  // the destructor isn't run).
  bool VisitCXXDeleteExpr(clang::CXXDeleteExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;

    const Expr* delete_arg = expr->getArgument()->IgnoreParenImpCasts();
    // We always delete a pointer, so do one dereference to get the
    // actual type being deleted.
    const Type* delete_ptr_type = GetTypeOf(delete_arg);
    const Type* delete_type = RemovePointerFromType(delete_ptr_type);
    if (CanIgnoreType(delete_ptr_type) && CanIgnoreType(delete_type))
      return true;

    if (delete_type && !IsPointerOrReferenceAsWritten(delete_type))
      ReportTypeUse(CurrentLoc(), delete_type);

    return true;
  }

  // Handle the case of passing references to variadic functions
  // (those with '...').  We need the full type information for the
  // reference in that case, since compilers seem to just deref the
  // var before passing it in.  Note we subclass all the
  // function-calling methods rather than HandleFunctionCall, because
  // a) we need type-specific caller information anyway, and b)
  // HandleFunctionCall isn't called for calls via function-pointers,
  // which we want.
  void ReportIfReferenceVararg(Expr** args, unsigned num_args,
                               const FunctionProtoType* callee_type) {
    if (callee_type && callee_type->isVariadic()) {
      const unsigned first_vararg_index = callee_type->getNumArgs();
      for (unsigned i = first_vararg_index; i < num_args; i++) {
        // We only care about reporting for references, but it's ok if
        // we catch a few non-ref types too (it's just redundant).
        // All expressions that are references will have their
        // valuekind be an LValue, so we use that as the test.
        if (args[i]->getValueKind() == clang::VK_LValue) {
          // The types of expressions 'see through' the reference to
          // the underlying type, which is exactly what we want here.
          ReportTypeUse(CurrentLoc(), GetTypeOf(args[i]));
        }
      }
    }
  }
  void ReportIfReferenceVararg(Expr** args, unsigned num_args,
                               const FunctionDecl* callee) {
    if (callee) {
      const FunctionProtoType* callee_type =
          DynCastFrom(callee->getType().getTypePtr());
      CHECK_(callee_type &&
             "The type of a FunctionDecl must be a FunctionProtoType.");
      ReportIfReferenceVararg(args, num_args, callee_type);
    }
  }

  // We only need visitors for CallExpr, ConstructExpr, and NewExpr
  // (which also captures their subclasses).  We can ignore DeleteExpr
  // since destructors never have arguments.  NewExpr we treat below,
  // since it requires other checks as well.
  bool VisitCallExpr(clang::CallExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;
    // Nothing to do if the called function is an old K&R-style function.
    const FunctionType* fn_type = GetCalleeFunctionType(expr);
    if (const FunctionProtoType* fn_proto = DynCastFrom(fn_type))
      ReportIfReferenceVararg(expr->getArgs(), expr->getNumArgs(), fn_proto);
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;
    ReportIfReferenceVararg(expr->getArgs(), expr->getNumArgs(),
                            expr->getConstructor());
    return true;
  }

  // An OverloadExpr is an overloaded function (or method) in an
  // uninstantiated template, that can't be resolved until the
  // template is instantiated.  The simplest case is something like:
  //    void Foo(int) { ... }
  //    void Foo(float) { ... }
  //    template<typename T> Fn(T t) { Foo(t); }
  // But by far the most common case is when the function-to-be-called
  // is also a templated function:
  //    template<typename T> Fn1(T t) { ... }
  //    template<typename T> Fn2(T t) { Fn1(t); }
  // In either case, we look at all the potential overloads.  If they
  // all exist in the same file -- which is pretty much always the
  // case, especially with a template calling a template -- we can do
  // an iwyu warning now, even without knowing the exact overload.
  // In that case, we store the fact we warned, so we won't warn again
  // when the template is instantiated.
  // TODO(csilvers): to be really correct, we should report *every*
  // overload that callers couldn't match via ADL.
  bool VisitOverloadExpr(clang::OverloadExpr* expr) {
    // No CanIgnoreCurrentASTNode() check here!  It's later in the function.

    // Make sure all overloads are in the same file.
    if (expr->decls_begin() == expr->decls_end())   // not sure this is possible
      return true;
    const NamedDecl* first_decl = *expr->decls_begin();
    const FileEntry* first_decl_file_entry = GetFileEntry(first_decl);
    for (OverloadExpr::decls_iterator it = expr->decls_begin();
         it != expr->decls_end(); ++it) {
      if (GetFileEntry(*it) != first_decl_file_entry)
        return true;
    }

    // For now, we're only worried about function calls.
    // TODO(csilvers): are there other kinds of overloads we need to check?
    const FunctionDecl* arbitrary_fn_decl = NULL;
    for (OverloadExpr::decls_iterator it = expr->decls_begin();
         it != expr->decls_end(); ++it) {
      const NamedDecl* decl = *it;
      // Sometimes a UsingShadowDecl comes between us and the 'real' decl.
      if (const UsingShadowDecl* using_shadow_decl = DynCastFrom(decl))
        decl = using_shadow_decl->getTargetDecl();
      if (const FunctionDecl* fn_decl = DynCastFrom(decl)) {
        arbitrary_fn_decl = fn_decl;
        break;
      } else if (const FunctionTemplateDecl* tpl_decl = DynCastFrom(decl)) {
        arbitrary_fn_decl = tpl_decl->getTemplatedDecl();
        break;
      }
    }

    // If we're an overloaded operator, we can never do the iwyu check
    // before instantiation-time, because we don't know if we might
    // end up being the built-in form of the operator.  (Even if the
    // only operator==() we see is in foo.h, we don't need to #include
    // foo.h if the only call to operator== we see is on two integers.)
    if (arbitrary_fn_decl && !arbitrary_fn_decl->isOverloadedOperator()) {
      processed_overload_locs_.insert(CurrentLoc());
      VERRS(7) << "Adding to processed_overload_locs: "
               << PrintableCurrentLoc() << "\n";
      // Because processed_overload_locs_ might be set in one visitor
      // but used in another, each with a different definition of
      // CanIgnoreCurrentASTNode(), we have to be conservative and set
      // the has-considered flag always.  But of course we only
      // actually report the function use if CanIgnoreCurrentASTNode()
      // is *currently* false.
      if (!CanIgnoreCurrentASTNode())
        ReportDeclUse(CurrentLoc(), arbitrary_fn_decl);
    }
    return true;
  }

  // TODO(csilvers): handle some special cases when we're a
  // CXXDependentScopeMemberExpr (e.g. vector<T>::resize().).  If the
  // base class is a TemplateSpecializationType, get its TemplateDecl
  // and if all explicit specializations and patterns are defined in
  // the same file, treat it as an expr with only one decl.  May have
  // trouble with methods defined in a different file than they're
  // declared.

  // If getOperatorNew() returns NULL, it means the operator-new is
  // overloaded, and technically we can't know which operator-new is
  // being called until the template is instantiated.  But if it looks
  // like a placement-new, we handle it at template-writing time
  // anyway.
  bool VisitCXXNewExpr(clang::CXXNewExpr* expr) {
    // Like in VisitOverloadExpr(), we set processed_overload_locs_
    // regardless of the value of CanIgnoreCurrentASTNode().

    // We say it's placement-new if the (lone) placment-arg is a
    // pointer.  Unfortunately, often clang will just say it's a
    // dependent type.  In that case, we can still say it's a pointer
    // in the (common) case the placement arg looks like '&something'.
    // (This is possibly wrong for classes that override operator&, but
    // those classes deserve what they get.)
    if (!expr->getOperatorNew() &&
        expr->getNumPlacementArgs() == 1 &&
        (GetTypeOf(expr->getPlacementArg(0))->isPointerType() ||
         GetTypeOf(expr->getPlacementArg(0))->isArrayType() ||
         IsAddressOf(expr->getPlacementArg(0)))) {
      // Treat this like an OverloadExpr.
      processed_overload_locs_.insert(CurrentLoc());
      VERRS(7) << "Adding to processed_overload_locs (placement-new): "
               << PrintableCurrentLoc() << "\n";
      if (!CanIgnoreCurrentASTNode()) {
        // We have to 'make up' a full file path for 'new'.  We'll
        // parse it to '<new>' before using, so any path that does
        // that, and is clearly a c++ path, is fine; its exact
        // contents don't matter that much.
        const FileEntry* use_file = CurrentFileEntry();
        preprocessor_info().FileInfoFor(use_file)->ReportFullSymbolUse(
            CurrentLoc(), "/usr/include/c++/<version>/new", "operator new");
      }
    }

    // We also need to do a varargs check, like for other function calls.
    if (CanIgnoreCurrentASTNode())  return true;
    ReportIfReferenceVararg(expr->getConstructorArgs(),
                            expr->getNumConstructorArgs(),
                            expr->getConstructor());
    return true;
  }

  // When we call (or potentially call) a function, do an IWYU check
  // via ReportDeclUse() to make sure the definition of the function
  // is properly #included.
  bool HandleFunctionCall(FunctionDecl* callee, const Type* parent_type) {
    if (!Base::HandleFunctionCall(callee, parent_type))
      return false;
    if (!callee || CanIgnoreCurrentASTNode() || CanIgnoreDecl(callee))
      return true;
    // We may have already been checked in a previous
    // VisitOverloadExpr() call.  Don't check again in that case.
    if (Contains(processed_overload_locs_, CurrentLoc()))
      return true;

    // TODO(csilvers): if the function is not inline, call
    // ReportDeclForwardDeclareUse() instead.
    ReportDeclUse(CurrentLoc(), callee);
    return true;
  }

  //------------------------------------------------------------
  // Visitors of types derived from clang::Type.

  bool VisitType(clang::Type* type) {
    // In VisitFunctionDecl(), we say all children of function
    // declarations are forward-declarable.  This is true, *except*
    // for the exception (throw) types.  We clean that up here.
    // TODO(csilvers): figure out how to do these two steps in one place.
    const FunctionProtoType* fn_type = NULL;
    if (!fn_type) {
      fn_type = current_ast_node()->template GetParentAs<FunctionProtoType>();
    }
    if (!fn_type) {
      if (const FunctionDecl* fn_decl
          = current_ast_node()->template GetParentAs<FunctionDecl>())
        fn_type = dyn_cast<FunctionProtoType>(GetTypeOf(fn_decl));
    }
    if (fn_type) {
      for (FunctionProtoType::exception_iterator it =
               fn_type->exception_begin();
           it != fn_type->exception_end(); ++it)
        if (it->getTypePtr() == type) {  // *we're* an exception decl
          current_ast_node()->set_in_forward_declare_context(false);
          break;
        }
    }

    return Base::VisitType(type);
  }

  bool VisitTemplateSpecializationType(
      clang::TemplateSpecializationType* type) {
    if (CanIgnoreCurrentASTNode())  return true;

    // We don't care about any of this if we ourselves are
    // fwd-declarable.
    if (CanForwardDeclareType(current_ast_node())) {
      current_ast_node()->set_in_forward_declare_context(true);
      return true;
    }

    return true;
  }

  //------------------------------------------------------------
  // Visitors defined by BaseAstVisitor.

  bool VisitNestedNameSpecifier(NestedNameSpecifier* nns) {
    if (!Base::VisitNestedNameSpecifier(nns))  return false;
    // If we're in an nns (e.g. the Foo in Foo::bar), we're never
    // forward-declarable, even if we're part of a pointer type, or in
    // a template argument, or whatever.
    current_ast_node()->set_in_forward_declare_context(false);
    return true;
  }

  // Template arguments are forward-declarable by default.  However,
  // default template template args shouldn't be: we're responsible for
  // the full type info for default args.  So no forward-declaring
  // MyClass in 'template<template<typename A> class T = MyClass> C ...'
  // We detect because MyClass's parent is TemplateTemplateParmDecl.
  // TODO(csilvers): And not when they're a type that's in
  // known_fully_used_tpl_type_args_.  See if that solves the problem with
  // I1_TemplateClass<std::vector<I1_Class> > i1_nested_templateclass(...)
  void DetermineForwardDeclareStatusForTemplateArg(ASTNode* ast_node) {
    const TemplateArgument* arg = ast_node->GetAs<TemplateArgument>();
    CHECK_(arg && "Should only pass in a template arg to DFDSFTA");

    if (!IsDefaultTemplateTemplateArg(ast_node)) {
      ast_node->set_in_forward_declare_context(true);
      return;
    }
    // We do allow *passed-in* template args to be forward-declared
    // when they're in a typedef.  That is, for code like this:
    //    template<class T> struct MyStruct { typedef pair<T, T> MyPair; };
    // we allow T to be forward-declared.  That's because the typedef
    // is not 're-exporting' the type: if anything, it was given the type.
    if (arg->getKind() == TemplateArgument::Type &&
        isa<SubstTemplateTypeParmType>(arg->getAsType().getTypePtr()) &&
        ast_node->HasAncestorOfType<TypedefDecl>()) {
      ast_node->set_in_forward_declare_context(true);
    }
  }

  bool VisitTemplateArgument(const TemplateArgument& arg) {
    if (!Base::VisitTemplateArgument(arg))  return false;
    // Template arguments are forward-declarable...usually.
    DetermineForwardDeclareStatusForTemplateArg(current_ast_node());
    return true;
  }

  bool VisitTemplateArgumentLoc(const TemplateArgumentLoc& argloc) {
    if (!Base::VisitTemplateArgumentLoc(argloc))  return false;
    // Template arguments are forward-declarable...usually.
    DetermineForwardDeclareStatusForTemplateArg(current_ast_node());
    return true;
  }

  //------------------------------------------------------------
  // Helper routines for visiting and traversing.  These helpers
  // encode the logic of whether a particular type of object
  // can be forward-declared or not.

  bool CanForwardDeclareTemplateName(const ASTNode* ast_node) const {
    CHECK_(ast_node->IsA<TemplateName>());
    // If we're a template template arg (A in template<template<class
    // T> class A>), we may be forward-declarable, or we may not.
    // Unfortunately, it's a lot of machinery to check, and we almost
    // always will be -- if you're passing in a template template,
    // aren't you going to use it?  You *could* do something like:
    // 'template<template<class T> class A> class C { A<int>* x; };'
    // and never dereference x, but that's pretty unlikely.  So for
    // now, we just assume template template args -- these are
    // TemplateName's whose parent is a TemplateArg -- always need
    // full type info.
    // TODO(csilvers): Add them to AddTypelikeTemplateArgTo instead,
    // and change InstantiatedTemplateVisitor to look for them being
    // used.  Then we can get rid of this if statement below.
    if (IsDefaultTemplateTemplateArg(ast_node))
      return false;

    // If we're in a forward-declare context, we can forward declare.  Duh...
    if (ast_node->in_forward_declare_context())
      return true;

    // It may not be possible for the parent to be something other
    // than a TemplateSpecializationType.  In any case, the remaining
    // heuristics depend on that.
    if (!ast_node->ParentIsA<TemplateSpecializationType>())
      return false;

    // Another place we disregard what the language allows: if we're
    // a dependent type, in theory we can be forward-declared.  But
    // we require the full definition anyway, so all the template
    // callers don't have to provide it instead.  Thus we don't
    // run the following commented-out code (left here for reference):
    //if (ast_node->GetParentAs<TemplateSpecializationType>()->isDependentType())
    //  return true;

    // If grandparent is a pointer (parent is
    // TemplateSpecializationType), we can forward-declare this name.
    if (!ast_node->AncestorIsA<PointerType>(2) &&
        !ast_node->AncestorIsA<LValueReferenceType>(2))
      return false;
    return true;
  }

  bool CanForwardDeclareType(const ASTNode* ast_node) const {
    CHECK_(ast_node->IsA<Type>());
    // If we're in a forward-declare context, well then, there you have it.
    if (ast_node->in_forward_declare_context())
      return true;
    // If we're in a typedef, we don't want to forward-declare even if
    // we're a pointer.  ('typedef Foo* Bar; Bar x; x->a' needs full
    // type of Foo.)
    if (ast_node->ParentIsA<TypedefDecl>())
      return false;

    // Read past elaborations like 'class' keyword or namespaces.
    while (ast_node->ParentIsA<ElaboratedType>()) {
      ast_node = ast_node->parent();
    }
    const Type* parent_type = ast_node->GetParentAs<Type>();
    return parent_type && IsPointerOrReferenceAsWritten(parent_type);
  }

 protected:
  const IwyuPreprocessorInfo& preprocessor_info() const {
    return preprocessor_info_;
  }

  const set<SourceLocation>& processed_overload_locs() {
    return processed_overload_locs_;
  }

  void ExtendProcessedOverloadLocs(const set<SourceLocation>& locs) {
    InsertAllInto(locs, &processed_overload_locs_);
  }

 private:
  template <typename T> friend class IwyuBaseAstVisitor;

  // Information gathered at preprocessor time, including #include info.
  const IwyuPreprocessorInfo& preprocessor_info_;

  // When we see an overloaded function that depends on a template
  // parameter, we can't resolve the overload until the template
  // is instantiated (e.g., MyFunc<int> in the following example):
  //    template<typename T> MyFunc() { OverloadedFunction(T()); }
  // However, sometimes we can do iwyu even before resolving the
  // overload, if *all* potential overloads live in the same file.  We
  // mark the location of such 'early-processed' functions here, so
  // when we see the function again at template-instantiation time, we
  // know not to do iwyu-checking on it again.  (Since the actual
  // function-call exprs are different between the uninstantiated and
  // instantiated calls, we can't store the exprs themselves, but have
  // to store their location.)
  set<SourceLocation> processed_overload_locs_;
};


// ----------------------------------------------------------------------
// --- InstantiatedTemplateVisitor
// ----------------------------------------------------------------------
//
// This class is used to find all template-specified types used in an
// instantiated template class, function, or method -- or rather, all
// such types that are used in a way that can't be forward-declared.
// That is, for
//    template<class T, class U> int Myfunc() { T* t; U u; Thirdclass z; }
// if we saw an instantiation such as myfunc<Foo, Bar>, we would pass
// that instantiation to this traversal class, and it would report
// that Bar is used in a non-forward-declarable way.  (It would not
// report Foo, which is used only in a forward-declarable way, and
// would not report Thirdclass, which is not a type specified in a
// template.)
//
// This class has two main entry points: one for instantiated
// template functions and methods (including static methods,
// constructor calls, and operator overloads), and one for
// instantiated template classes.
//
// In each case, it is given the appropriate node from the AST that
// captures the instantiation (a TemplateSpecializationType or
// CallExpr), and returns a set of Type* nodes of types that are used
// in a non-forward-declarable way.  Note it's safe to call this even
// on non-templatized functions and classes; we'll just always return
// the empty set in that case.
//
// The traversal of the AST is done via RecursiveASTVisitor, which uses
// CRTP (http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
// TODO(csilvers): move this to its own file?

class InstantiatedTemplateVisitor
    : public IwyuBaseAstVisitor<InstantiatedTemplateVisitor> {
 public:
  typedef IwyuBaseAstVisitor<InstantiatedTemplateVisitor> Base;

  InstantiatedTemplateVisitor(CompilerInstance* compiler,
                              const IwyuPreprocessorInfo& preprocessor_info)
      : Base(compiler, preprocessor_info) {
    Clear();
  }

  //------------------------------------------------------------
  // Public entry points

  // ScanInstantiatedFunction() looks through the template definition of
  // the given function as well as the definitions of all functions
  // called from it (directly or indirectly) and records all template
  // type arguments fully used by them and all methods used by them.
  // The "fully used type arguments" are a subset of
  // tpl_type_args_of_interest, which are the types we care about, and
  // usually explicitly written at the call site.
  //
  // ScanInstantiatedType() is similar, except that it looks through
  // the definition of a class template instead of a statement.

  void ScanInstantiatedFunction(
      const FunctionDecl* fn_decl, const Type* parent_type,
      const SourceLocation caller_loc,
      const set<SourceLocation>& processed_overload_locs,
      const set<const Type*>& tpl_type_args_of_interest) {
    Clear();
    caller_loc_ = caller_loc;
    ExtendProcessedOverloadLocs(processed_overload_locs);  // copy from caller
    tpl_type_args_of_interest_ = tpl_type_args_of_interest;
    TraverseExpandedTemplateFunctionHelper(fn_decl, parent_type);
  }

  // This isn't a Stmt, but sometimes we need to fully instantiate
  // a template class to get at a field of it, for instance:
  // MyClass<T>::size_type s;
  void ScanInstantiatedType(
      const Type* type, SourceLocation caller_loc,
      const set<SourceLocation>& processed_overload_locs,
      const set<const Type*>& tpl_type_args_of_interest) {
    Clear();
    caller_loc_ = caller_loc;
    ExtendProcessedOverloadLocs(processed_overload_locs);  // copy from caller
    tpl_type_args_of_interest_ = tpl_type_args_of_interest;

    // As in TraverseExpandedTemplateFunctionHelper, we ignore all AST nodes
    // that will be reported when we traverse the uninstantiated type.
    if (const NamedDecl* type_decl_as_written = TypeToDeclAsWritten(type)) {
      AstFlattenerVisitor nodeset_getter(compiler());
      nodes_to_ignore_ = nodeset_getter.GetNodesBelow(
          const_cast<NamedDecl*>(type_decl_as_written));
    }

    TraverseType(QualType(type, 0));
  }

  //------------------------------------------------------------
  // Implements virtual methods from Base.

  // When checking a template instantiation, we don't care where the
  // template definition is, so we never have any reason to ignore a
  // node.
  virtual bool CanIgnoreCurrentASTNode() const {
    return nodes_to_ignore_.Contains(*current_ast_node());
  }

  // For template instantiations, we want to print the symbol even if
  // it's not from the main compilation unit.
  virtual bool ShouldPrintSymbolFromCurrentFile() const {
     return GetVerboseLevel() >= 5;
  }

  virtual string GetSymbolAnnotation() const { return " in tpl"; }

  // We only care about types that are Subst types, and also are in
  // tpl_type_args_of_interest_.
  virtual bool CanIgnoreType(const Type* type) const {
    if (nodes_to_ignore_.Contains(type))
      return true;
    const SubstTemplateTypeParmType* subst_type = DynCastFrom(type);
    if (!subst_type)
      return true;
    const Type* real_type = subst_type->getReplacementType().getTypePtr();
    return GetMatchingTypesOfInterest(real_type).empty();
  }

  // We ignore function calls in nodes_to_ignore_, which were already
  // handled by the template-as-written, and function names that we
  // are not responsible for because the template code is (for
  // instance, we're not responsible for a vector's call to
  // allocator::allocator(), because <vector> provides it for us).
  virtual bool CanIgnoreDecl(const Decl* decl) const {
    return nodes_to_ignore_.Contains(decl);
  }

  // We always attribute type uses to the template instantiator.  For
  // decls, we do unless it looks like the template "intends to
  // provide" the decl, by #including the file that defines the decl
  // (if templates call other templates, we have to find the right
  // template).
  virtual void ReportDeclUse(SourceLocation used_loc, const NamedDecl* decl) {
    const SourceLocation actual_used_loc = GetLocOfTemplateThatProvides(decl);
    if (actual_used_loc.isValid()) {
      // If a template is responsible for this decl, then we don't add
      // it to the cache; the cache is only for decls that the
      // original caller is responsible for.
      Base::ReportDeclUse(actual_used_loc, decl);
    } else {
      // Let all the currently active types and decls know about this
      // report, so they can update their cache entries.
      for (Each<CacheStoringScope*> it(&cache_storers_); !it.AtEnd(); ++it)
        (*it)->NoteReportedDecl(decl);
      Base::ReportDeclUse(caller_loc_, decl);
    }
  }

  virtual void ReportTypeUse(SourceLocation used_loc, const Type* type) {
    // clang desugars template types, so Foo<MyTypedef>() gets turned
    // into Foo<UnderlyingType>().  Try to convert back.
    type = ResugarType(type);
    for (Each<CacheStoringScope*> it(&cache_storers_); !it.AtEnd(); ++it)
      (*it)->NoteReportedType(type);
    Base::ReportTypeUse(caller_loc_, type);
  }

  //------------------------------------------------------------
  // Overridden traverse-style methods from Base.

  // The 'convenience' HandleFunctionCall is perfect for us!
  bool HandleFunctionCall(FunctionDecl* callee, const Type* parent_type) {
    // clang desugars template types, so Foo<MyTypedef>() gets turned
    // into Foo<UnderlyingType>().  Try to convert back.
    parent_type = ResugarType(parent_type);
    if (!Base::HandleFunctionCall(callee, parent_type))
      return false;
    if (!callee || CanIgnoreCurrentASTNode())  return true;
    return TraverseExpandedTemplateFunctionHelper(callee, parent_type);
  }

  bool TraverseSizeOfAlignOfExpr(clang::SizeOfAlignOfExpr* expr) {
    if (!Base::TraverseSizeOfAlignOfExpr(expr))  return false;
    if (CanIgnoreCurrentASTNode())  return true;
    const Type* arg_type = expr->getTypeOfArgument().getTypePtr();
    // Calling sizeof on a reference-to-X is the same as calling it on X.
    if (const ReferenceType* reftype = DynCastFrom(arg_type)) {
      arg_type = reftype->getPointeeTypeAsWritten().getTypePtr();
    }
    if (const TemplateSpecializationType* type = DynCastFrom(arg_type)) {
      // Even though sizeof(MyClass<T>) only requires knowing how much
      // storage MyClass<T> takes, the language seems to require that
      // MyClass<T> be fully instantiated, even typedefs.  (Try
      // compiling 'template<class T> struct C { typedef typename T::a t; };
      // class S; int main() { return sizeof(C<S>); }'.)
      return TraverseDataAndTypeMembersOfClassHelper(type);
    }
    return true;
  }

  bool TraverseTemplateSpecializationType(
      clang::TemplateSpecializationType* type) {
    if (!Base::TraverseTemplateSpecializationType(type))  return false;
    if (CanIgnoreCurrentASTNode())  return true;
    return TraverseDataAndTypeMembersOfClassHelper(type);
  }


  // These do the actual work of finding the types to return.  Our
  // task is made easier since (at least in theory), every time we
  // instantiate a template type, the instantiation has type
  // SubstTemplateTypeParmTypeLoc in the AST tree.
  bool VisitSubstTemplateTypeParmType(clang::SubstTemplateTypeParmType* type) {
    if (CanIgnoreCurrentASTNode())  return true;
    // Ignore everything not in our list of explicitly-typed-in-code types.
    const Type* actual_type = type->getReplacementType().getTypePtr();
    const set<const Type*> types_of_interest  // almost always has 0 or 1 entry
        = GetMatchingTypesOfInterest(actual_type);
    if (types_of_interest.empty())
      return Base::VisitSubstTemplateTypeParmType(type);

    // If we're a nested-name-specifier (the Foo in Foo::bar), we need
    // our full type info no matter what the context (even if we're a
    // pointer, or a template arg, or whatever).
    // TODO(csilvers): consider encoding this logic via
    // in_forward_declare_context.  I think this will require changing
    // in_forward_declare_context to yes/no/maybe.
    if (current_ast_node()->ParentIsA<NestedNameSpecifier>()) {
      ReportTypesUse(CurrentLoc(), types_of_interest);
      return Base::VisitSubstTemplateTypeParmType(type);
    }

    // sizeof(a reference type) is the same as sizeof(underlying type).
    // We have to handle that specially here, or else we'll say the
    // reference is forward-declarable, below.
    if (current_ast_node()->ParentIsA<SizeOfAlignOfExpr>() &&
        isa<ReferenceType>(actual_type)) {
      // This is a bit tricky: we can't call
      // ReportTypesUse(..., types_of_interest), because we want a
      // dereferenced version of what's in types_of_interest.  We
      // can't just use actual_type because it's possibly desugared.
      const ReferenceType* actual_reftype = cast<ReferenceType>(actual_type);
      const Type* deref_actual_type
          = actual_reftype->getPointeeTypeAsWritten().getTypePtr();
      const Type* desugared_deref_actual_type
          = deref_actual_type->getUnqualifiedDesugaredType();
      for (Each<const Type*> it(&types_of_interest); !it.AtEnd(); ++it) {
        if (const ReferenceType* reftype = DynCastFrom(*it)) {
          const Type* deref = reftype->getPointeeTypeAsWritten().getTypePtr();
          if (deref->getUnqualifiedDesugaredType()
              == desugared_deref_actual_type)
            ReportTypeUse(CurrentLoc(), deref);
        }
      }
      return Base::VisitSubstTemplateTypeParmType(type);
    }

    // If we're used in a forward-declare context (MyFunc<T>() { T* t; }),
    // or are ourselves a pointer type (MyFunc<Myclass*>()),
    // we don't need to do anything: we're fine being forward-declared.
    if (current_ast_node()->in_forward_declare_context())
      return Base::VisitSubstTemplateTypeParmType(type);

    if (current_ast_node()->ParentIsA<PointerType>() ||
        current_ast_node()->ParentIsA<LValueReferenceType>() ||
        IsPointerOrReferenceAsWritten(actual_type))
      return Base::VisitSubstTemplateTypeParmType(type);

    // We attribute all uses in an instantiated template to the
    // template's caller.
    // TODO(csilvers): If the parent is a TemplateSpecializationType,
    // then we need to figure out how the parent is being used.
    ReportTypesUse(caller_loc_, types_of_interest);
    return Base::VisitSubstTemplateTypeParmType(type);
  }

  // If constructing an object, check the type we're constructing.
  // Normally we'd see that type later, when traversing the return
  // type of the constructor-decl, but if we wait for that, we'll lose
  // any SubstTemplateTypeParmType's we have (we lose all
  // SubstTemplateTypeParmType's going from Expr to Decl).
  // TODO(csilvers): This should maybe move to HandleFunctionCall.
  bool VisitCXXConstructExpr(clang::CXXConstructExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;
    const set<const Type*> types_of_interest
        = GetMatchingTypesOfInterest(GetTypeOf(expr));
    ReportTypesUse(caller_loc_, types_of_interest);
    return Base::VisitCXXConstructExpr(expr);
  }

 private:
  // Clears the state of the visitor.
  void Clear() {
    caller_loc_ = SourceLocation();
    tpl_type_args_of_interest_.clear();
    traversed_decls_.clear();
    nodes_to_ignore_.clear();
    cache_storers_.clear();
  }

  // If we see the instantiated template using a type or decl (such as
  // std::allocator), we want to know if the template-as-written is
  // providing the type or decl, so the code using the instantiated
  // template doesn't have to.  For instance:
  //    vector<int, /*allocator<int>*/> v;   // in foo.cc
  // Does <vector> provide the definition of allocator<int>?  If not,
  // foo.cc will have to #include <allocator>.
  //   We say the template-as-written does provide the decl if it,
  // or any other header seen since we started instantiating the
  // template, sees it.  The latter requirement is to deal with
  // template args that cross instantiation boundaries: if we have a
  // templated class that #includes "foo.h" and has a scoped_ptr<Foo>,
  // we say the templated class provides Foo, even though it's
  // scoped_ptr.h that's actually trying to call Foo::Foo and ::~Foo.
  SourceLocation GetLocOfTemplateThatProvides(const NamedDecl* decl) const {
    for (const ASTNode* ast_node = current_ast_node(); ast_node;
         ast_node = ast_node->parent()) {
      if (preprocessor_info().PublicHeaderIntendsToProvide(
              GetFileEntry(ast_node->GetLocation()),
              GetFileEntry(decl)))
        return ast_node->GetLocation();
    }
    return SourceLocation();   // an invalid source-loc
  }

  bool SomeInstantiatedTemplateIntendsToProvide(const NamedDecl* decl) const {
    return GetLocOfTemplateThatProvides(decl).isValid();
  }

  // The type that gets substituted in SubstTemplateTypeParmTypeLoc is
  // a fully desugared type -- typedefs followed, etc.  So testing
  // whether it's the same as one of the user-specified template types
  // is non-trivial.  We find all the user-specified types that could
  // qualify, a bit-overconservative, but fine in practice.
  //    We also say our type is of interest if it's a template type
  // with a template param in tpl_type_args_of_interest_, but *only*
  // if we (the current file) aren't responsible for providing this
  // template type.  This means we don't say we're interested in
  // allocator<Foo> (which vector provides), but we are interested
  // in hash<Foo> (which hash_set doesn't provide).
  set<const Type*> GetMatchingTypesOfInterest(const Type* type) const {
    set<const Type*> retval;
    const Type* canonical_type = type->getUnqualifiedDesugaredType();
    for (Each<const Type*> it(&tpl_type_args_of_interest_); !it.AtEnd(); ++it) {
      if ((*it)->getUnqualifiedDesugaredType() == canonical_type)
        retval.insert(*it);
    }
    if (const RecordType* record_type = DynCastFrom(canonical_type)) {
      if (const ClassTemplateSpecializationDecl* tpl_decl =
          DynCastFrom(record_type->getDecl())) {
        if (!SomeInstantiatedTemplateIntendsToProvide(tpl_decl)) {
          const TemplateArgumentList& tpl_args = tpl_decl->getTemplateArgs();
          for (unsigned i = 0; i < tpl_args.size(); ++i) {
            const TemplateArgument& arg = tpl_args[i];
            // TODO(csilvers): deal with other kinds of template args
            // (TemplateTemplateArgs, and const expressions like sizeof(T))
            if (arg.getKind() != TemplateArgument::Type)
              continue;
            const Type* arg_type = arg.getAsType().getTypePtr();
            const Type* canonical_arg_type
                = arg_type->getUnqualifiedDesugaredType();
            for (Each<const Type*> it(&tpl_type_args_of_interest_);
                 !it.AtEnd(); ++it) {
              if ((*it)->getUnqualifiedDesugaredType() == canonical_arg_type) {
                retval.insert(record_type);
                return retval;   // nothing else will affect retval
              }
            }
          }
        }
      }
    }

    return retval;
  }

  // clang desugars template types, so Foo<MyTypedef>() gets turned
  // into Foo<UnderlyingType>().  We can 'resugar' using
  // tpl_type_args_of_interest_.  If tpl_type_args_of_interest_ shows
  // nothing interesting, we return the type under the input
  // SubstTemplateTypeParmType.  If the given type isn't a
  // SubstTemplateTypeParmType, just return it unchanged.
  const Type* ResugarType(const Type* type) const {
    if (type && isa<SubstTemplateTypeParmType>(type)) {
      const set<const Type*> sugared_types = GetMatchingTypesOfInterest(type);
      // It's possible for two sugared types to have mapped to the
      // same unsugared type, but for our purposes they're equivalent,
      // so we just pick one arbitrarily.
      if (!sugared_types.empty())
        return *sugared_types.begin();
      return RemoveSubstTemplateTypeParm(type);
    }
    return type;
  }

  bool TraverseExpandedTemplateFunctionHelper(const FunctionDecl* fn_decl,
                                              const Type* parent_type) {
    if (!fn_decl || Contains(traversed_decls_, fn_decl))
      return true;   // avoid recursion and repetition
    traversed_decls_.insert(fn_decl);

    // If we have cached the reporting done for this decl before,
    // report again (but with the new caller_loc this time).
    // Otherwise, for all reporting done in the rest of this scope,
    // store in the cache for this function.
    if (ReplayUsesFromCache(function_calls_full_use_cache_,
                            fn_decl, caller_loc_))
      return true;
    // Make sure all the types we report in the recursive TraverseDecl
    // calls, below, end up in the cache for fn_decl.
    CacheStoringScope css(&cache_storers_, &function_calls_full_use_cache_,
                          fn_decl, tpl_type_args_of_interest_);

    // We want to ignore all nodes that are the same in this
    // instantiated function as they are in the uninstantiated version
    // of the function.  The latter will be reported when we traverse
    // the uninstantiated function, so we don't need to re-traverse
    // them here.
    AstFlattenerVisitor nodeset_getter(compiler());
    // This gets to the decl for the (uninstantiated) template-as-written:
    const FunctionDecl* decl_as_written
        = fn_decl->getTemplateInstantiationPattern();
    if (!decl_as_written) {
      if (fn_decl->isImplicit()) {     // TIP not set up for implicit methods
        // TODO(csilvers): handle implicit template methods
      } else {                         // not a templated decl
        decl_as_written = fn_decl;
      }
    }
    if (decl_as_written) {
      FunctionDecl* const daw = const_cast<FunctionDecl*>(decl_as_written);
      nodes_to_ignore_.AddAll(nodeset_getter.GetNodesBelow(daw));
    }

    // We need to iterate over the function.  We do so even if it's
    // an implicit function.
    if (fn_decl->isImplicit()) {
      if (!TraverseImplicitDeclHelper(const_cast<FunctionDecl*>(fn_decl)))
        return false;
    } else {
      if (!TraverseDecl(const_cast<FunctionDecl*>(fn_decl)))
        return false;
    }

    // If we're a constructor, we also need to construct the entire class,
    // even typedefs that aren't used at construct time. Try compiling
    //    template<class T> struct C { typedef typename T::a t; };
    //    class S; int main() { C<S> c; }
    if (isa<CXXConstructorDecl>(fn_decl)) {
      CHECK_(parent_type && "How can a constructor have no parent?");
      if (!TraverseDataAndTypeMembersOfClassHelper(
              dyn_cast<TemplateSpecializationType>(parent_type)))
        return false;
    }
    return true;
  }

  // Does the actual recursing over data members and type members of
  // the instantiated class.  Unlike
  // TraverseClassTemplateSpecializationDecl() in the base class, it
  // does *not* traverse the methods.
  bool TraverseDataAndTypeMembersOfClassHelper(
      const TemplateSpecializationType* type) {
    // No point in doing traversal if we're not fully instantiated.
    if (!type || type->isDependentType())
      return true;

    const ClassTemplateSpecializationDecl* class_decl
        = DynCastFrom(TypeToDeclAsWritten(type));
    CHECK_(class_decl && "TemplateSpecializationType is not a TplSpecDecl?");
    if (Contains(traversed_decls_, class_decl))
      return true;   // avoid recursion & repetition
    traversed_decls_.insert(class_decl);

    // If we have cached the reporting done for this decl before,
    // report again (but with the new caller_loc this time).
    // Otherwise, for all reporting done in the rest of this scope,
    // store in the cache for this function.
    if (ReplayUsesFromCache(class_members_full_use_cache_,
                            class_decl, caller_loc_))
      return true;
    if (ReplayClassMemberUsesFromPrecomputedList(type))
      return true;

    // Make sure all the types we report in the recursive TraverseDecl
    // calls, below, end up in the cache for class_decl.
    CacheStoringScope css(&cache_storers_, &class_members_full_use_cache_,
                          class_decl, tpl_type_args_of_interest_);

    for (DeclContext::decl_iterator it = class_decl->decls_begin();
         it != class_decl->decls_end(); ++it) {
      if (isa<CXXMethodDecl>(*it) || isa<FunctionTemplateDecl>(*it))
        continue;
      if (!TraverseDecl(*it))
        return false;
    }
    return true;
  }

  //------------------------------------------------------------
  // Cache methods.  Caches hold the list of full uses found when we
  // last instantiated a given decl, saving a lot of tree-walking if
  // we have to do it again.

  // Returns true if we replayed uses, false if key isn't in the cache.
  bool ReplayUsesFromCache(const FullUseCache& cache, const NamedDecl* key,
                           SourceLocation use_loc) {
    if (!cache.Contains(key, tpl_type_args_of_interest_))
      return false;
    VERRS(6) << "(Replaying full-use information from the cache for "
             << key->getQualifiedNameAsString() << ")\n";
    ReportTypesUse(use_loc,
                   cache.GetFullUseTypes(key, tpl_type_args_of_interest_));
    ReportDeclsUse(use_loc,
                   cache.GetFullUseDecls(key, tpl_type_args_of_interest_));
    return true;
  }

  // We precompute (hard-code) results of calling
  // TraverseDataAndTypeMembersOfClassHelper for some types (mostly
  // STL types).  This way we don't even need to traverse them once.
  // Returns true iff we did appropriate reporting for this type.
  bool ReplayClassMemberUsesFromPrecomputedList(
      const TemplateSpecializationType* tpl_type) {
    CHECK_(!tpl_type->isDependentType() && "Replay only instantiated types");
    if (current_ast_node() && current_ast_node()->in_forward_declare_context())
      return true;   // never depend on any types if a fwd-decl

    const set<const Type*>& fulluse_types =
        FullUseCache::GetPrecomputedUnsugaredFullUseTypes(preprocessor_info(),
                                                          tpl_type);
    if (!fulluse_types.empty()) {
      VERRS(6) << "(Using pre-computed list of full-use information for "
               << TypeToDeclAsWritten(tpl_type)->getQualifiedNameAsString()
               << ")\n";
      for (Each<const Type*> it(&fulluse_types); !it.AtEnd(); ++it) {
        ReportTypesUse(caller_loc_, GetMatchingTypesOfInterest(*it));
      }
      return true;
    }
    return false;
  }

  //------------------------------------------------------------
  // Member variables.

  // Where the template is instantiated.
  SourceLocation caller_loc_;

  // The types mentioned in the call expression/etc -- those types
  // actually typed by the user (or inferred template arguments in
  // template function calls).  It excludes default tpl parameters.
  set<const Type*> tpl_type_args_of_interest_;

  // Used to avoid recursion in the *Helper() methods.
  set<const Decl*> traversed_decls_;

  AstFlattenerVisitor::NodeSet nodes_to_ignore_;

  // The current set of nodes we're updating cache entries for.
  set<CacheStoringScope*> cache_storers_;

  // These caches record what types and decls we reported when
  // instantiating a particular decl.  That avoids extra work if we
  // see the same decl again -- we can replay those reports, just from
  // a new caller_loc.
  static FullUseCache function_calls_full_use_cache_;
  static FullUseCache class_members_full_use_cache_;
};  // class InstantiatedTemplateVisitor

FullUseCache InstantiatedTemplateVisitor::function_calls_full_use_cache_;
FullUseCache InstantiatedTemplateVisitor::class_members_full_use_cache_;


// ----------------------------------------------------------------------
// --- IwyuAstConsumer
// ----------------------------------------------------------------------
//
// This class listens to Clang's events as the AST is generated.
//
// The traversal of the AST is done via RecursiveASTVisitor, which uses
// CRTP (http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)

class IwyuAstConsumer
    : public ASTConsumer, public IwyuBaseAstVisitor<IwyuAstConsumer> {
 public:
  typedef IwyuBaseAstVisitor<IwyuAstConsumer> Base;

  IwyuAstConsumer(CompilerInstance* compiler,
                  const IwyuPreprocessorInfo& preprocessor_info)
      : Base(compiler, preprocessor_info),
        instantiated_template_visitor_(compiler, preprocessor_info) {}

  //------------------------------------------------------------
  // Implements pure virtual methods from Base.

  // Returns true if we are not interested in symbols used in used_in
  // for whatever reason.  For instance, we can ignore nodes that are
  // neither in the file we're compiling nor in its associated .h file.
  virtual bool CanIgnoreCurrentASTNode() const {
    if (!ShouldReportIWYUViolationsFor(CurrentFileEntry()))
      return true;               // ignore symbols used outside foo.{h,cc}

    // If we're a field of a typedef type, ignore us: our rule is that
    // the author of the typedef is responsible for everything
    // involving the typedef.
    if (IsMemberOfATypedef(current_ast_node()))
      return true;

    return false;
  }

  // We print symbols from files in the main compilation unit (foo.cc,
  // foo.h, foo-inl.h) if the debug level is 5 or 6, for non-system
  // files if the debug level is 7, and all files if the debug level
  // is 8 or more.
  virtual bool ShouldPrintSymbolFromCurrentFile() const {
    return ShouldPrintSymbolFromFile(CurrentFileEntry());
  }

  virtual string GetSymbolAnnotation() const { return ""; }

  // We are interested in all types for iwyu checking.
  virtual bool CanIgnoreType(const Type* type) const {
    return type == NULL;
  }

  virtual bool CanIgnoreDecl(const Decl* decl) const {
    return decl == NULL;
  }

  //------------------------------------------------------------
  // Parser event handlers.  Clang will call them to notify this
  // ASTConsumer as it parses the source code.  See class ASTConsumer in
  // clang/AST/ASTConsumer.h
  // for all the handlers we can override.

  // Called once at the beginning of the compilation.
  virtual void Initialize(ASTContext& context) {}  // NOLINT

  // Called once at the end of the compilation.
  virtual void HandleTranslationUnit(ASTContext& context) {  // NOLINT
    // TODO(csilvers): automatically detect preprocessing is done, somehow.
    const_cast<IwyuPreprocessorInfo*>(&preprocessor_info())->
        HandlePreprocessingDone();

    TraverseDecl(context.getTranslationUnitDecl());

    // We have to calculate the .h files before the .cc file, since
    // the .cc file inherits #includes from the .h files, and we
    // need to figure out what those #includes are going to be.
    const FileEntry* const main_file = preprocessor_info().main_file();
    const set<const FileEntry*>* const files_to_report_iwyu_violations_for
        = preprocessor_info().files_to_report_iwyu_violations_for();
    for (Each<const FileEntry*> file(files_to_report_iwyu_violations_for);
         !file.AtEnd(); ++file) {
      if (*file == main_file)
        continue;
      CHECK_(preprocessor_info().FileInfoFor(*file));
      preprocessor_info().FileInfoFor(*file)
          ->CalculateAndReportIwyuViolations();
    }
    CHECK_(preprocessor_info().FileInfoFor(main_file));
    preprocessor_info().FileInfoFor(main_file)
        ->CalculateAndReportIwyuViolations();

    exit(1);  // we need to force the compile to fail so we can re-run.
  }

  //------------------------------------------------------------
  // AST visitors.  We start by adding a visitor callback for
  // most of the subclasses of Decl/Stmt/Type listed in:
  //   clang/AST/DeclNodes.def
  //   clang/AST/StmtNodes.td
  //   clang/AST/TypeNodes.def
  // We exclude only:
  //   1) abstract declarations and types with no logic (e.g. NamedDecl)
  //   2) ObjC declarations, statements, and types (e.g. ObjcIvarDecl)
  // RecursiveASTVisitor defines specialized visitors for each specific
  // math operation (MulAssign, OffsetOf, etc).  We don't override
  // those callbacks, but use their default behavior, which is to call
  // back to VisitUnaryOperator, VisitBinaryOperator, etc.
  //
  // Over time, as we understand when a callback is called and
  // which can be ignored by iwyu, we will pare down the list.
  //   Each of these returns a bool: false if we want to abort the
  // traversal (we never do).  For Visit*(), we can abort early if
  // we're not in the main compilation-unit, since we only ever give
  // iwyu warnings on symbols in those files.

  // --- Visitors of types derived from clang::Decl.

  bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl* decl) {
    if (CanIgnoreCurrentASTNode())  return true;
    ReportDeclUse(CurrentLoc(), decl->getNamespace());
    return Base::VisitNamespaceAliasDecl(decl);
  }

  bool VisitUsingDecl(clang::UsingDecl* decl) {
    if (CanIgnoreCurrentASTNode())  return true;
    // The shadow decls hold the declarations for the var/fn/etc we're
    // using.  (There may be more than one if, say, we're using an
    // overloaded function.)  We check to make sure nothing we're
    // using is an iwyu violation.
    for (UsingDecl::shadow_iterator it = decl->shadow_begin();
         it != decl->shadow_end(); ++it) {
      ReportDeclForwardDeclareUse(CurrentLoc(), (*it)->getTargetDecl());
    }
    return Base::VisitUsingDecl(decl);
  }

  bool VisitTagDecl(clang::TagDecl* decl) {
    if (CanIgnoreCurrentASTNode())  return true;
    // If it's not a definition, and it's not a friend declaration, it
    // must be a forward-declaration.  Don't count the 'inline'
    // forward-declares like 'int foo(class T* t) ...'
    // TODO(csilvers): replace IsDeclNodeInsideFriend with IsFriendDecl().
    if (!decl->isDefinition() && !IsDeclNodeInsideFriend(current_ast_node()) &&
        !decl->isEmbeddedInDeclarator()) {
      // If we're a templated class, make sure we add the whole template.
      const NamedDecl* decl_to_fwd_declare = decl;
      if (const CXXRecordDecl* cxx_decl = DynCastFrom(decl))
        if (cxx_decl->getDescribedClassTemplate())
          decl_to_fwd_declare = cxx_decl->getDescribedClassTemplate();
      preprocessor_info().FileInfoFor(CurrentFileEntry())->AddForwardDeclare(
          decl_to_fwd_declare);

      // A forward declaration is not a "use" of a forward-declaration
      // (a "use" is when you use a decl, not when you declare it), so
      // we don't need to report this.  However, there are some
      // situations where we don't want to remove the forward
      // declaration, even if it's not used anywhere.  If the
      // forward-decl has a linkage spec ('extern "C"') or has
      // gcc-style __attributes__, then it can't be removed, since
      // that information probably isn't encoded anywhere else.  To
      // make sure iwyu doesn't remove this decl, we claim it's used.
      // This is a bit of a hack; better would be to have an API that
      // says, "don't remove this decl even if it's not used."
      if (current_ast_node()->ParentIsA<LinkageSpecDecl>() || decl->hasAttrs())
        ReportDeclForwardDeclareUse(CurrentLoc(), decl_to_fwd_declare);

      // If we're a nested class ("class A { class SubA; };"), then we
      // can't necessary be removed either, since we're part of the
      // public API of the enclosing class.  So again, fake a use.
      // However, multiple declarations of the nested class aren't
      // needed.  So we only need to 'fake' the use of one of them; we
      // prefer the one that's actually the definition, if present.
      // TODO(csilvers): repeat this logic in VisitClassTemplateDecl().
      if (current_ast_node()->ParentIsA<CXXRecordDecl>() ||
          // For templated nested-classes, a ClassTemplateDecl is interposed.
          (current_ast_node()->ParentIsA<ClassTemplateDecl>() &&
           current_ast_node()->AncestorIsA<CXXRecordDecl>(2))) {
        // Prefer the definition if present -- but only if it's
        // defined inside the class, like we are.
        const clang::NamedDecl* canonical_decl = decl->getDefinition();
        if (!canonical_decl || canonical_decl->isOutOfLine()) {
          // If not, just take an arbitrary, but fixed, redecl (that
          // is, every redecl will map to the same place).  Note these
          // must be inline (only definitions can be out of line).
          canonical_decl = GetNonfriendClassRedecl(decl);
        }
        if (decl == canonical_decl)  // we're the redecl iwyu should keep!
          ReportDeclForwardDeclareUse(CurrentLoc(), decl_to_fwd_declare);
      }
    }
    return Base::VisitTagDecl(decl);
  }

  // If you specialize a template that is only declared, we need
  // to keep the declaration around.  That is, for code like this:
  //    template <class T> struct Foo;
  //    template<> struct Foo<int> { ... };
  // we don't want iwyu to recommend removing the 'forward declare' of Foo.
  bool VisitClassTemplateSpecializationDecl(
      clang::ClassTemplateSpecializationDecl* decl) {
    if (CanIgnoreCurrentASTNode())  return true;
    ClassTemplateDecl* specialized_decl = decl->getSpecializedTemplate();
    if (GetDefinitionForClass(specialized_decl) == NULL)
      ReportDeclForwardDeclareUse(CurrentLoc(), specialized_decl);
    return Base::VisitClassTemplateSpecializationDecl(decl);
  }

  // If you say 'typedef Foo Bar', then clients can use Bar however
  // they want without having to worry about #including anything
  // except you.  That puts you on the hook for all the #includes that
  // Bar might need, for *anything* one might want to do to a Bar.
  // TODO(csilvers): we can probably relax this rule in .cc files.
  // TODO(csilvers): this should really move into IwyuBaseASTVisitor
  // (that way we'll correctly identify need for hash<> in hash_set).
  // This is a Traverse*() because Visit*() can't call HandleFunctionCall().
  bool TraverseTypedefDecl(clang::TypedefDecl* decl) {
    // Before we go up the tree, make sure the parents know we don't
    // forward-declare the underlying type of a typedef decl.
    current_ast_node()->set_in_forward_declare_context(false);
    if (!Base::TraverseTypedefDecl(decl))
      return false;
    if (CanIgnoreCurrentASTNode())  return true;

    const Type* underlying_type = decl->getUnderlyingType().getTypePtr();
    const Decl* underlying_decl = TypeToDeclAsWritten(underlying_type);

    // We simulate a user calling all the methods in a class.
    if (const CXXRecordDecl* record_decl = DynCastFrom(underlying_decl)) {
      for (DeclContext::decl_iterator it = record_decl->decls_begin();
           it != record_decl->decls_end(); ++it) {
        FunctionDecl* fn_decl = NULL;
        if (CXXMethodDecl* method_decl = DynCastFrom(*it)) {
          fn_decl = method_decl;
        } else if (FunctionTemplateDecl* tpl_decl = DynCastFrom(*it)) {
          fn_decl = tpl_decl->getTemplatedDecl();   // templated method decl
        } else {
          continue;    // not a method or static method
        }
        if (!this->getDerived().HandleFunctionCall(fn_decl, underlying_type))
          return false;
      }
    }
    // We don't have to simulate a user instantiating the type, because
    // RecursiveASTVisitor.h will recurse on the typedef'ed type for us.
    return true;
  }

  // --- Visitors of types derived from clang::Stmt.

  // Called whenever a variable, function, enum, etc is used.
  bool VisitDeclRefExpr(clang::DeclRefExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;
    ReportDeclUse(CurrentLoc(), expr->getDecl());
    return Base::VisitDeclRefExpr(expr);
  }

  // The compiler fully instantiates a template class before taking
  // the size of it.  So so do we.
  bool VisitSizeOfAlignOfExpr(clang::SizeOfAlignOfExpr* expr) {
    if (CanIgnoreCurrentASTNode())  return true;

    const Type* arg_type = expr->getTypeOfArgument().getTypePtr();
    // Calling sizeof on a reference-to-X is the same as calling it on X.
    if (const ReferenceType* reftype = DynCastFrom(arg_type)) {
      arg_type = reftype->getPointeeTypeAsWritten().getTypePtr();
    }
    const set<const Type*> tpl_type_args = GetExplicitTplTypeArgsOf(arg_type);
    if (IsTemplatizedType(arg_type)) {
      instantiated_template_visitor_.ScanInstantiatedType(
          arg_type, CurrentLoc(), processed_overload_locs(), tpl_type_args);
    }

    return Base::VisitSizeOfAlignOfExpr(expr);
  }

  // --- Visitors of types derived from clang::Type.

  bool VisitTypedefType(clang::TypedefType* type) {
    if (CanIgnoreCurrentASTNode())  return true;
    // TypedefType::getDecl() returns the place where the typedef is defined.
    ReportDeclUse(CurrentLoc(), type->getDecl());
    return Base::VisitTypedefType(type);
  }

  // This is a superclass of RecordType and CXXRecordType.
  bool VisitTagType(clang::TagType* type) {
    if (CanIgnoreCurrentASTNode())  return true;

    // If we're forward-declarable, then no complicated checking is
    // needed: just forward-declare.  If we're already elaborated
    // ('class Foo x') but not namespace-qualified ('class ns::Foo x')
    // there's no need even to forward-declare!
    if (CanForwardDeclareType(current_ast_node())) {
      current_ast_node()->set_in_forward_declare_context(true);
      if (!IsElaborationNode(current_ast_node()->parent()) ||
          IsNamespaceQualifiedNode(current_ast_node()->parent())) {
        ReportDeclForwardDeclareUse(CurrentLoc(), type->getDecl());
      }
      return Base::VisitTagType(type);
    }

    // OK, seems to be a use that requires the full type.
    ReportDeclUse(CurrentLoc(), type->getDecl());
    return Base::VisitTagType(type);
  }

  // Like for CXXConstructExpr, etc., we sometimes need to instantiate
  // a class when looking at TemplateSpecializationType -- for instance,
  // when we need to access a class typedef: MyClass<A>::value_type.
  bool VisitTemplateSpecializationType(
      clang::TemplateSpecializationType* type) {
    if (CanIgnoreCurrentASTNode())  return true;

    // If we're not in a forward-declare context, use of a template
    // specialization requires having the full type information.
    if (!CanForwardDeclareType(current_ast_node())) {
      const set<const Type*> tpl_type_args = GetExplicitTplTypeArgsOf(type);
      instantiated_template_visitor_.ScanInstantiatedType(
          type, CurrentLoc(), processed_overload_locs(), tpl_type_args);
    }

    return Base::VisitTemplateSpecializationType(type);
  }

  // --- Visitors defined by BaseASTVisitor (not RecursiveASTVisitor).

  bool VisitTemplateName(TemplateName template_name) {
    if (CanIgnoreCurrentASTNode())  return true;
    if (!Base::VisitTemplateName(template_name))  return false;
    if (const TemplateDecl* tpl_decl = template_name.getAsTemplateDecl()) {
      if (CanForwardDeclareTemplateName(current_ast_node())) {
        current_ast_node()->set_in_forward_declare_context(true);
        ReportDeclForwardDeclareUse(CurrentLoc(), tpl_decl);
      } else {
        current_ast_node()->set_in_forward_declare_context(false);
        ReportDeclUse(CurrentLoc(), tpl_decl);
      }
    }
    return true;
  }

  // For expressions that require us to instantiate a template
  // (CallExpr of a template function, or CXXConstructExpr of a
  // template class, etc), we need to instantiate the template and
  // check IWYU status of the template parameters *in the template
  // code* (so for 'MyFunc<T>() { T t; ... }', the contents of
  // MyFunc<MyClass> add an iwyu requirement on MyClass).
  bool HandleFunctionCall(FunctionDecl* callee, const Type* parent_type) {
    if (!Base::HandleFunctionCall(callee, parent_type))
      return false;
    if (!callee || CanIgnoreCurrentASTNode() || CanIgnoreDecl(callee))
      return true;

    // Figure out the template parameters for this function or method,
    // if any.  For methods, add in template args *explicitly*
    // specified when the template class was created.
    set<const Type*> tpl_type_args = GetTplTypeArgsOfFunction(callee);
    if (parent_type)     // means we're a method of a class
      InsertAllInto(GetExplicitTplTypeArgsOf(parent_type), &tpl_type_args);

    if (IsTemplatizedFunctionDecl(callee) || IsTemplatizedType(parent_type))
      instantiated_template_visitor_.ScanInstantiatedFunction(
          callee, parent_type,
          CurrentLoc(), processed_overload_locs(), tpl_type_args);
    return true;
  }

 private:
  // Class we call to handle instantiated template functions and classes.
  InstantiatedTemplateVisitor instantiated_template_visitor_;
};  // class IwyuAstConsumer

// We use an ASTFrontendAction to hook up IWYU with Clang.
class IwyuAction : public ASTFrontendAction {
 protected:
  virtual ASTConsumer* CreateASTConsumer(CompilerInstance& compiler,  // NOLINT
                                         llvm::StringRef /* dummy */) {
    // Do this first thing after getting our hands on a CompilerInstance.
    InitGlobals(&compiler.getSourceManager(),
                &compiler.getPreprocessor().getHeaderSearchInfo());

    // Also init the globals that are local to this file.
    g_explicitly_instantiated_classes.clear();

    IwyuPreprocessorInfo* const preprocessor_consumer
        = new IwyuPreprocessorInfo();
    IwyuAstConsumer* const ast_consumer
        = new IwyuAstConsumer(&compiler, *preprocessor_consumer);

    compiler.getPreprocessor().addPPCallbacks(preprocessor_consumer);
    return ast_consumer;
  }
};


} // namespace include_what_you_use


// Everything below is adapted from clang/examples/clang-interpreter/main.cpp.

#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/system_error.h"

using clang::ASTFrontendAction;
using clang::CompilerInstance;
using clang::CompilerInvocation;
using clang::Diagnostic;
using clang::DiagnosticIDs;
using clang::DiagnosticOptions;
using clang::TextDiagnosticPrinter;
using clang::driver::ArgStringList;
using clang::driver::Command;
using clang::driver::Compilation;
using clang::driver::Driver;
using clang::driver::JobList;
using llvm::IntrusiveRefCntPtr;
using llvm::LLVMContext;
using llvm::OwningPtr;
using llvm::SmallString;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::MemoryBuffer;
using llvm::StringRef;
using llvm::errs;
using llvm::llvm_shutdown;
using llvm::raw_svector_ostream;
using llvm::sys::getHostTriple;
using llvm::sys::Path;
using include_what_you_use::IwyuAction;
using include_what_you_use::StartsWith;
using std::set;
using std::string;

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
Path GetExecutablePath(const char *Argv0) {
  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *main_addr = (void*) (intptr_t) GetExecutablePath;
  return Path::GetMainExecutable(Argv0, main_addr);
}

static const char *SaveStringInSet(std::set<std::string> &SavedStrings,
                                   StringRef S) {
  return SavedStrings.insert(S).first->c_str();
}

static void ExpandArgsFromBuf(const char *Arg,
                              SmallVectorImpl<const char*> &ArgVector,
                              set<string> &SavedStrings) {
  const char *FName = Arg + 1;
  OwningPtr<MemoryBuffer> MemBuf;
  if (MemoryBuffer::getFile(FName, MemBuf)) {
    ArgVector.push_back(SaveStringInSet(SavedStrings, Arg));
    return;
  }

  const char *Buf = MemBuf->getBufferStart();
  char InQuote = ' ';
  string CurArg;

  for (const char *P = Buf; ; ++P) {
    if (*P == '\0' || (isspace(*P) && InQuote == ' ')) {
      if (!CurArg.empty()) {

        if (CurArg[0] != '@') {
          ArgVector.push_back(SaveStringInSet(SavedStrings, CurArg));
        } else {
          ExpandArgsFromBuf(CurArg.c_str(), ArgVector, SavedStrings);
        }

        CurArg = "";
      }
      if (*P == '\0')
        break;
      else
        continue;
    }

    if (isspace(*P)) {
      if (InQuote != ' ')
        CurArg.push_back(*P);
      continue;
    }

    if (*P == '"' || *P == '\'') {
      if (InQuote == *P)
        InQuote = ' ';
      else if (InQuote == ' ')
        InQuote = *P;
      else
        CurArg.push_back(*P);
      continue;
    }

    if (*P == '\\') {
      ++P;
      if (*P != '\0')
        CurArg.push_back(*P);
      continue;
    }
    CurArg.push_back(*P);
  }
}

static void ExpandArgv(int argc, const char **argv,
                       SmallVectorImpl<const char*> &ArgVector,
                       set<string> &SavedStrings) {
  for (int i = 0; i < argc; ++i) {
    const char *Arg = argv[i];
    if (Arg[0] != '@') {
      ArgVector.push_back(SaveStringInSet(SavedStrings, string(Arg)));
      continue;
    }

    ExpandArgsFromBuf(Arg, ArgVector, SavedStrings);
  }
}

int main(int argc, const char **argv) {
  void* main_addr = (void*) (intptr_t) GetExecutablePath;
  Path path = GetExecutablePath(argv[0]);
  TextDiagnosticPrinter* diagnostic_client =
    new TextDiagnosticPrinter(errs(), DiagnosticOptions());

  IntrusiveRefCntPtr<DiagnosticIDs> diagnostic_id(new DiagnosticIDs());
  Diagnostic diagnostics(diagnostic_id, diagnostic_client);
  Driver driver(path.str(), getHostTriple(), "a.out",
                false, false, diagnostics);
  driver.setTitle("include what you use");

  // Expand out any response files passed on the command line
  set<string> SavedStrings;
  SmallVector<const char*, 256> args;

  ExpandArgv(argc, argv, args, SavedStrings);

  // FIXME: This is a hack to try to force the driver to do something we can
  // recognize. We need to extend the driver library to support this use model
  // (basically, exactly one input, and the operation mode is hard wired).
  args.push_back("-fsyntax-only");
  OwningPtr<Compilation> compilation(driver.BuildCompilation(args.size(),
                                                             args.data()));
  if (!compilation)
    return 0;

  // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const JobList& jobs = compilation->getJobs();
  if (jobs.size() != 1 || !isa<Command>(*jobs.begin())) {
    SmallString<256> msg;
    raw_svector_ostream out(msg);
    compilation->PrintJob(out, compilation->getJobs(), "; ", true);
    diagnostics.Report(clang::diag::err_fe_expected_compiler_job) << out.str();
    return 1;
  }

  const Command *command = cast<Command>(*jobs.begin());
  if (StringRef(command->getCreator().getName()) != "clang") {
    diagnostics.Report(clang::diag::err_fe_expected_clang_command);
    return 1;
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const ArgStringList &cc_arguments = command->getArguments();
  const char** args_start = const_cast<const char**>(cc_arguments.data());
  const char** args_end = args_start + cc_arguments.size();
  OwningPtr<CompilerInvocation> invocation(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*invocation, args_start, args_end,
                                     diagnostics);
  invocation->getFrontendOpts().DisableFree = false;

  // Show the invocation, with -v.
  if (invocation->getHeaderSearchOpts().Verbose) {
    errs() << "clang invocation:\n";
    compilation->PrintJob(errs(), compilation->getJobs(), "\n", true);
    errs() << "\n";
  }

  // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

  // Create a compiler instance to handle the actual work.
  CompilerInstance compiler;
  compiler.setInvocation(invocation.take());

  // Create the compilers actual diagnostics engine.
  compiler.createDiagnostics(args_end - args_start, args_start);
  if (!compiler.hasDiagnostics())
    return 1;

  // Infer the builtin include path if unspecified.
  if (compiler.getHeaderSearchOpts().UseBuiltinIncludes &&
      compiler.getHeaderSearchOpts().ResourceDir.empty())
    compiler.getHeaderSearchOpts().ResourceDir =
      CompilerInvocation::GetResourcesPath(argv[0], main_addr);

  // Create and execute the frontend to generate an LLVM bitcode module.
  OwningPtr<ASTFrontendAction> action(new IwyuAction);
  if (!compiler.ExecuteAction(*action))
    return 1;

  llvm_shutdown();

  // We always return a failure exit code, to indicate we didn't
  // successfully compile (produce a .o for) the source files we were
  // given.
  return 1;
}
