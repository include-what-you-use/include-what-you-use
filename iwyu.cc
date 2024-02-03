//===--- iwyu.cc - main logic and driver for include-what-you-use ---------===//
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

#include <cstdio>
#include <cstdlib>                      // for atoi, exit
#include <functional>
#include <map>                          // for map, swap, etc
#include <memory>                       // for unique_ptr
#include <set>                          // for set, set<>::iterator, swap
#include <string>                       // for string, operator+, etc
#include <utility>                      // for pair
#include <vector>                       // for vector, swap

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExprConcepts.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/TemplateName.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileEntry.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TypeTraits.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Sema.h"
#include "iwyu_ast_util.h"
#include "iwyu_cache.h"
#include "iwyu_driver.h"
#include "iwyu_globals.h"
#include "iwyu_location_util.h"
#include "iwyu_output.h"
#include "iwyu_port.h"  // for CHECK_
#include "iwyu_preprocessor.h"
#include "iwyu_stl_util.h"
#include "iwyu_use_flags.h"
#include "iwyu_verrs.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"

// TODO: Clean out pragmas as IWYU improves.
// IWYU pragma: no_include "clang/AST/Redeclarable.h"
// IWYU pragma: no_include "clang/AST/StmtIterator.h"
// IWYU pragma: no_include "clang/AST/UnresolvedSet.h"
// IWYU pragma: no_include "clang/Basic/CustomizableOptional.h"
// IWYU pragma: no_include "clang/Lex/PPCallbacks.h"
// IWYU pragma: no_include "llvm/ADT/iterator.h"
// IWYU pragma: begin_keep
namespace clang {
class PPCallbacks;
}  // namespace clang
// IWYU pragma: end_keep

namespace clang {
class FriendDecl;
class Stmt;

namespace driver {
class ToolChain;
}
}  // namespace clang

namespace include_what_you_use {

// I occasionally clean up this list by running:
// $ grep "using clang":: iwyu.cc | cut -b14- | tr -d ";" | while read t; do grep -q "[^:]$t" iwyu.cc || echo $t; done
using clang::ASTConsumer;
using clang::ASTContext;
using clang::ASTFrontendAction;
using clang::ArraySubscriptExpr;
using clang::Attr;
using clang::BinaryOperator;
using clang::CXXBaseSpecifier;
using clang::CXXBindTemporaryExpr;
using clang::CXXCatchStmt;
using clang::CXXConstructExpr;
using clang::CXXConstructorDecl;
using clang::CXXDefaultArgExpr;
using clang::CXXDeleteExpr;
using clang::CXXDestructorDecl;
using clang::CXXForRangeStmt;
using clang::CXXMemberCallExpr;
using clang::CXXMethodDecl;
using clang::CXXNewExpr;
using clang::CXXOperatorCallExpr;
using clang::CXXRecordDecl;
using clang::CXXTemporaryObjectExpr;
using clang::CXXTypeidExpr;
using clang::CallExpr;
using clang::CastExpr;
using clang::ClassTemplateSpecializationDecl;
using clang::CompilerInstance;
using clang::ConceptSpecializationExpr;
using clang::Decl;
using clang::DeclContext;
using clang::DeclRefExpr;
using clang::DecltypeType;
using clang::DeducedTemplateSpecializationType;
using clang::ElaboratedType;
using clang::ElaboratedTypeKeyword;
using clang::ElaboratedTypeLoc;
using clang::EnumConstantDecl;
using clang::EnumDecl;
using clang::EnumType;
using clang::Expr;
using clang::FriendDecl;
using clang::FriendTemplateDecl;
using clang::FunctionDecl;
using clang::FunctionProtoType;
using clang::FunctionTemplateDecl;
using clang::FunctionType;
using clang::LateParsedTemplate;
using clang::LinkageSpecDecl;
using clang::MemberExpr;
using clang::NamedDecl;
using clang::NamespaceAliasDecl;
using clang::NestedNameSpecifier;
using clang::NestedNameSpecifierLoc;
using clang::OptionalFileEntryRef;
using clang::OverloadExpr;
using clang::PPCallbacks;
using clang::ParmVarDecl;
using clang::PointerType;
using clang::Preprocessor;
using clang::QualType;
using clang::QualifiedTypeLoc;
using clang::RecordDecl;
using clang::RecursiveASTVisitor;
using clang::ReferenceType;
using clang::Sema;
using clang::SourceLocation;
using clang::SourceManager;
using clang::Stmt;
using clang::SubstTemplateTypeParmType;
using clang::TagDecl;
using clang::TagType;
using clang::TemplateArgument;
using clang::TemplateArgumentList;
using clang::TemplateArgumentLoc;
using clang::TemplateDecl;
using clang::TemplateName;
using clang::TemplateSpecializationType;
using clang::TemplateSpecializationTypeLoc;
using clang::TemplateTypeParmDecl;
using clang::TranslationUnitDecl;
using clang::Type;
using clang::TypeAliasDecl;
using clang::TypeAliasTemplateDecl;
using clang::TypeLoc;
using clang::TypeSourceInfo;
using clang::TypeTrait;
using clang::TypeTraitExpr;
using clang::TypedefDecl;
using clang::TypedefNameDecl;
using clang::TypedefType;
using clang::UnaryExprOrTypeTraitExpr;
using clang::UsingDecl;
using clang::UsingDirectiveDecl;
using clang::UsingShadowDecl;
using clang::UsingType;
using clang::ValueDecl;
using clang::VarDecl;
using clang::driver::ToolChain;
using llvm::cast;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::errs;
using llvm::isa;
using std::map;
using std::set;
using std::string;
using std::swap;
using std::vector;

namespace {

bool CanIgnoreLocation(SourceLocation loc) {
  // If we're in a macro expansion, we always want to treat this as
  // being in the expansion location, never the as-written location,
  // since that's what the compiler does.  CanIgnoreCurrentASTNode()
  // is an optimization, so we want to be conservative about what we
  // ignore.
  OptionalFileEntryRef file_entry = GetFileEntry(loc);
  OptionalFileEntryRef file_entry_after_macro_expansion =
      GetFileEntry(GetInstantiationLoc(loc));

  // ignore symbols used outside foo.{h,cc} + check_also
  return (!ShouldReportIWYUViolationsFor(file_entry) &&
          !ShouldReportIWYUViolationsFor(file_entry_after_macro_expansion));
}

}  // anonymous namespace

enum class DerefKind { None, RemoveRefs, RemoveRefsAndPtr };

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
// 5) Add two callbacks that subclasses can override (just like any
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

template <class Derived>
class BaseAstVisitor : public RecursiveASTVisitor<Derived> {
 public:
  typedef RecursiveASTVisitor<Derived> Base;

  // We need to create implicit ctor/dtor nodes, which requires
  // non-const methods on CompilerInstance, so the var can't be const.
  explicit BaseAstVisitor(CompilerInstance* compiler)
      : compiler_(compiler),
        current_ast_node_(nullptr) {}

  virtual ~BaseAstVisitor() = default;

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
  // (1) Maintain current_ast_node_ and print each node.

  // How subclasses can access current_ast_node_;
  const ASTNode* current_ast_node() const {
    return current_ast_node_;
  }
  ASTNode* current_ast_node() {
    return current_ast_node_;
  }
  void set_current_ast_node(ASTNode* an) { current_ast_node_ = an; }

  bool TraverseDecl(Decl* decl) {
    if (!decl)
      return true;
    if (current_ast_node_ && current_ast_node_->StackContainsContent(decl))
      return true;               // avoid recursion
    ASTNode node(decl);
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName(GetKindName(decl)) << PrintablePtr(decl)
             << PrintableDecl(decl) << "\n";
    }
    return Base::TraverseDecl(decl);
  }

  bool TraverseStmt(Stmt* stmt) {
    if (!stmt)
      return true;
    if (current_ast_node_ && current_ast_node_->StackContainsContent(stmt))
      return true;               // avoid recursion
    ASTNode node(stmt);
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName(GetKindName(stmt)) << PrintablePtr(stmt)
             << PrintableStmt(stmt) << "\n";
    }
    return Base::TraverseStmt(stmt);
  }

  bool TraverseType(QualType qualtype) {
    if (qualtype.isNull())
      return true;
    const Type* type = qualtype.getTypePtr();
    if (current_ast_node_ && current_ast_node_->StackContainsContent(type))
      return true;               // avoid recursion
    ASTNode node(type);
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName(GetKindName(type)) << PrintablePtr(type)
             << PrintableType(type) << "\n";
    }
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
    if (typeloc.getAs<QualifiedTypeLoc>()) {
      typeloc = typeloc.getUnqualifiedLoc();
    }
    if (typeloc.isNull())
      return true;
    if (current_ast_node_ && current_ast_node_->StackContainsContent(&typeloc))
      return true;               // avoid recursion
    ASTNode node(&typeloc);
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName(GetKindName(typeloc)) << PrintableTypeLoc(typeloc)
             << "\n";
    }
    return Base::TraverseTypeLoc(typeloc);
  }

  bool TraverseNestedNameSpecifier(NestedNameSpecifier* nns) {
    if (!nns)
      return true;
    ASTNode node(nns);
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("NestedNameSpecifier")
             << PrintablePtr(nns) << PrintableNestedNameSpecifier(nns) << "\n";
    }
    if (!this->getDerived().VisitNestedNameSpecifier(nns))
      return false;
    return Base::TraverseNestedNameSpecifier(nns);
  }

  bool TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc nns_loc) {
    NestedNameSpecifier* nns = nns_loc.getNestedNameSpecifier();
    if (!nns)
      return true;
    ASTNode node(&nns_loc);
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("NestedNameSpecifier")
             << PrintablePtr(nns) << PrintableNestedNameSpecifier(nns) << "\n";
    }
    // TODO(csilvers): have VisitNestedNameSpecifierLoc instead.
    if (!this->getDerived().VisitNestedNameSpecifier(nns))
      return false;
    return Base::TraverseNestedNameSpecifierLoc(nns_loc);
  }

  bool TraverseTemplateName(TemplateName template_name) {
    if (template_name.isNull())
      return true;
    ASTNode node(&template_name);
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("TemplateName")
             << PrintableTemplateName(template_name) << "\n";
    }
    if (!this->getDerived().VisitTemplateName(template_name))
      return false;
    return Base::TraverseTemplateName(template_name);
  }

  bool TraverseTemplateArgument(const TemplateArgument& arg) {
    if (arg.isNull())
      return true;
    ASTNode node(&arg);
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("TemplateArgument")
             << PrintablePtr(&arg) << PrintableTemplateArgument(arg) << "\n";
    }
    if (!this->getDerived().VisitTemplateArgument(arg))
      return false;
    return Base::TraverseTemplateArgument(arg);
  }

  bool TraverseTemplateArgumentLoc(const TemplateArgumentLoc& argloc) {
    ASTNode node(&argloc);
    CurrentASTNodeUpdater canu(&current_ast_node_, &node);
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("TemplateArgumentLoc")
             << PrintablePtr(&argloc) << PrintableTemplateArgumentLoc(argloc)
             << "\n";
    }
    if (!this->getDerived().VisitTemplateArgumentLoc(argloc))
      return false;
    return Base::TraverseTemplateArgumentLoc(argloc);
  }

  // We have a few Visit methods that RecursiveASTVisitor does not. Provide
  // empty default implementations.
  bool VisitNestedNameSpecifier(NestedNameSpecifier*) {
    return true;
  }

  bool VisitTemplateName(TemplateName) {
    return true;
  }

  bool VisitTemplateArgument(const TemplateArgument&) {
    return true;
  }

  bool VisitTemplateArgumentLoc(const TemplateArgumentLoc&) {
    return true;
  }

  //------------------------------------------------------------
  // (2) Provide functions related to the current location.

  SourceLocation CurrentLoc() const {
    CHECK_(current_ast_node_ && "Call CurrentLoc within Visit* or Traverse*");
    return current_ast_node_->GetLocation();
  }

  string CurrentFilePath() const {
    return GetFilePath(CurrentLoc());
  }

  OptionalFileEntryRef CurrentFileEntry() const {
    return GetFileEntry(CurrentLoc());
  }

  string PrintableCurrentLoc() const {
    return PrintableLoc(CurrentLoc());
  }

  //------------------------------------------------------------
  // (3) Utilities for logging

  // The current file location, the class or decl or type name in
  // brackets, along with annotations such as the indentation depth,
  // etc.
  string AnnotatedName(const string& name) const {
    return (PrintableCurrentLoc() + ": (" +
            std::to_string(current_ast_node_->depth()) + GetSymbolAnnotation() +
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

  //------------------------------------------------------------
  // (4) Add implicit text.
  //
  // This simulates a call to the destructor of every non-POD field and base
  // class in all classes with destructors, to mark them as used by virtue of
  // being class members.
  bool TraverseCXXDestructorDecl(CXXDestructorDecl* decl) {
    if (!Base::TraverseCXXDestructorDecl(decl))
      return false;
    if (CanIgnoreCurrentASTNode())
      return true;
    // We only care about calls that are actually defined.
    if (!decl || !decl->isThisDeclarationADefinition())
      return true;

    // Collect all the fields (and bases) we destroy, and call the dtor.
    set<const Type*> member_types;
    const CXXRecordDecl* record = decl->getParent();
    for (RecordDecl::field_iterator it = record->field_begin();
         it != record->field_end(); ++it) {
      member_types.insert(it->getType().getTypePtr());
    }
    for (CXXRecordDecl::base_class_const_iterator it = record->bases_begin();
         it != record->bases_end(); ++it) {
      member_types.insert(it->getType().getTypePtr());
    }
    for (const Type* type : member_types) {
      const NamedDecl* member_decl = TypeToDeclAsWritten(type);
      // We only want those fields that are c++ classes.
      if (const CXXRecordDecl* cxx_field_decl = DynCastFrom(member_decl)) {
        if (const CXXDestructorDecl* field_dtor =
                cxx_field_decl->getDestructor()) {
          if (!this->getDerived().TraverseImplicitDestructorCall(
                  const_cast<CXXDestructorDecl*>(field_dtor), type))
            return false;
        }
      }
    }
    return true;
  }

  //------------------------------------------------------------
  // (5) Add TraverseImplicitDestructorCall and HandleFunctionCall.

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
  // the first argument to HandleFunctionCall may be nullptr (e.g. when
  // constructing a built-in type), in which case the handler should
  // immediately return.

  // If the function being called is a member of a class, parent_type
  // is the type of the method's owner (parent), as it is written in
  // the source.  (We need the type-as-written so we can distinguish
  // explicitly-written template args from default template args.)  We
  // also pass in the CallExpr (or CXXConstructExpr, etc).  This may
  // be nullptr if the function call is implicit.
  bool HandleFunctionCall(FunctionDecl* callee, const Type* parent_type,
                          const Expr* calling_expr) {
    if (!callee)
      return true;
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("FunctionCall")
             << PrintablePtr(callee) << PrintableDecl(callee) << "\n";
    }
    return true;
  }

  bool TraverseImplicitDestructorCall(CXXDestructorDecl* decl,
                                      const Type* type_being_destroyed) {
    if (CanIgnoreCurrentASTNode())
      return true;
    if (!decl)
      return true;
    if (ShouldPrintSymbolFromCurrentFile()) {
      errs() << AnnotatedName("Destruction")
             << PrintableType(type_being_destroyed) << "\n";
    }
    return this->getDerived().HandleFunctionCall(decl, type_being_destroyed,
                                                 static_cast<Expr*>(nullptr));
  }

  bool TraverseCallExpr(CallExpr* expr) {
    if (!Base::TraverseCallExpr(expr))
      return false;
    if (CanIgnoreCurrentASTNode())
      return true;
    return this->getDerived().HandleFunctionCall(expr->getDirectCallee(),
                                                 TypeOfParentIfMethod(expr),
                                                 expr);
  }

  bool TraverseCXXMemberCallExpr(CXXMemberCallExpr* expr) {
    if (!Base::TraverseCXXMemberCallExpr(expr))
      return false;
    if (CanIgnoreCurrentASTNode())
      return true;
    return this->getDerived().HandleFunctionCall(expr->getDirectCallee(),
                                                 TypeOfParentIfMethod(expr),
                                                 expr);
  }

  bool TraverseCXXOperatorCallExpr(CXXOperatorCallExpr* expr) {
    if (!Base::TraverseCXXOperatorCallExpr(expr))
      return false;
    if (CanIgnoreCurrentASTNode())
      return true;

    const Type* parent_type = TypeOfParentIfMethod(expr);
    // If we're a free function -- bool operator==(MyClass a, MyClass b) --
    // we still want to have a parent_type, as if we were defined as
    // MyClass::operator==.  So we go through the arguments and take the
    // first one that's a class, and associate the function with that.
    if (!parent_type) {
      if (const Expr* first_argument = GetFirstClassArgument(expr))
        parent_type = GetTypeOf(first_argument);
    }
    return this->getDerived().HandleFunctionCall(expr->getDirectCallee(),
                                                 parent_type, expr);
  }

  bool TraverseCXXConstructExpr(CXXConstructExpr* expr) {
    if (!Base::TraverseCXXConstructExpr(expr))
      return false;
    if (CanIgnoreCurrentASTNode())
      return true;

    if (!this->getDerived().HandleFunctionCall(expr->getConstructor(),
                                               GetTypeOf(expr),
                                               expr))
      return false;

    // When creating a local variable or a temporary, but not a pointer, the
    // constructor is also responsible for destruction (which happens
    // implicitly when the variable goes out of scope).  Only when initializing
    // a field of a class does the constructor not have to worry
    // about destruction.  It turns out it's easier to check for that.
    bool will_call_implicit_destructor_on_leaving_scope =
        !IsCXXConstructExprInInitializer(current_ast_node()) &&
        !IsCXXConstructExprInNewExpr(current_ast_node());
    if (will_call_implicit_destructor_on_leaving_scope) {
      if (const CXXDestructorDecl* dtor_decl = GetSiblingDestructorFor(expr)) {
        if (!this->getDerived().TraverseImplicitDestructorCall(
                const_cast<CXXDestructorDecl*>(dtor_decl), GetTypeOf(expr)))
          return false;
      }
    }
    return true;
  }

  bool TraverseCXXTemporaryObjectExpr(CXXTemporaryObjectExpr* expr) {
    if (!Base::TraverseCXXTemporaryObjectExpr(expr))
      return false;
    if (CanIgnoreCurrentASTNode())
      return true;

    // In this case, we *know* we're responsible for destruction as well.
    CXXConstructorDecl* ctor_decl = expr->getConstructor();
    CXXDestructorDecl* dtor_decl =
        const_cast<CXXDestructorDecl*>(GetSiblingDestructorFor(expr));
    const Type* type = GetTypeOf(expr);
    return (this->getDerived().HandleFunctionCall(ctor_decl, type, expr) &&
            this->getDerived().HandleFunctionCall(dtor_decl, type, expr));
  }

  bool TraverseCXXNewExpr(CXXNewExpr* expr) {
    if (!Base::TraverseCXXNewExpr(expr))
      return false;
    if (CanIgnoreCurrentASTNode())
      return true;

    const Type* parent_type = expr->getAllocatedType().getTypePtrOrNull();
    // 'new' calls operator new in addition to the ctor of the new-ed type.
    if (FunctionDecl* operator_new = expr->getOperatorNew()) {
      // If operator new is a method, it must (by the semantics of
      // per-class operator new) be a method on the class we're newing.
      const Type* op_parent = nullptr;
      if (isa<CXXMethodDecl>(operator_new))
        op_parent = parent_type;
      if (!this->getDerived().HandleFunctionCall(operator_new, op_parent, expr))
        return false;
    }
    return true;
  }

  bool TraverseCXXDeleteExpr(CXXDeleteExpr* expr) {
    if (!Base::TraverseCXXDeleteExpr(expr))
      return false;

    if (CanIgnoreCurrentASTNode())
      return true;

    const Type* parent_type = expr->getDestroyedType().getTypePtrOrNull();
    // We call operator delete in addition to the dtor of the deleted type.
    if (FunctionDecl* operator_delete = expr->getOperatorDelete()) {
      // If operator delete is a method, it must (by the semantics of per-
      // class operator delete) be a method on the class we're deleting.
      const Type* op_parent = nullptr;
      if (isa<CXXMethodDecl>(operator_delete))
        op_parent = parent_type;
      if (!this->getDerived().HandleFunctionCall(operator_delete, op_parent,
                                                 expr))
        return false;
    }
    const CXXDestructorDecl* dtor = GetDestructorForDeleteExpr(expr);
    return this->getDerived().HandleFunctionCall(
        const_cast<CXXDestructorDecl*>(dtor), parent_type, expr);
  }

  // This is to catch function pointers to templates.
  // For instance, 'MyFunctionPtr p = &TplFn<MyClass*>;': we need to
  // expand TplFn to see if it needs full type info for MyClass.
  bool TraverseDeclRefExpr(DeclRefExpr* expr) {
    if (!Base::TraverseDeclRefExpr(expr))
      return false;
    if (CanIgnoreCurrentASTNode())
      return true;

    if (FunctionDecl* fn_decl = DynCastFrom(expr->getDecl())) {
      if (!IsImplicitlyInstantiatedDfn(fn_decl))
        return true;
      if (const auto* call_expr =  // Skip intermediate ImplicitCastExpr node.
          current_ast_node_->GetAncestorAs<CallExpr>(2)) {
        if (call_expr->getDirectCallee() == fn_decl)
          return true;
      }
      // If fn_decl has a class-name before it -- 'MyClass::method' --
      // it's a method pointer.
      const Type* parent_type = nullptr;
      if (expr->getQualifier() && expr->getQualifier()->getAsType())
        parent_type = expr->getQualifier()->getAsType();
      if (!this->getDerived().HandleFunctionCall(fn_decl, parent_type, expr))
        return false;
    }
    return true;
  }

  /// Return whether this visitor should recurse into implicit
  /// code, e.g., implicit constructors and destructors.
  bool shouldVisitImplicitCode() const {
    return true;
  }

 protected:
  CompilerInstance* compiler() {
    return compiler_;
  }

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
      return ContainsKey(others, type);
    }
    bool Contains(const Decl* decl) const {
      return ContainsKey(others, decl);
    }
    bool Contains(const ASTNode& node) const {
      if (const TypeLoc* tl = node.GetAs<TypeLoc>()) {
        return ContainsValue(typelocs, *tl);
      } else if (const NestedNameSpecifierLoc* nl =
                     node.GetAs<NestedNameSpecifierLoc>()) {
        return ContainsValue(nnslocs, *nl);
      } else if (const TemplateName* tn = node.GetAs<TemplateName>()) {
        // The best we can do is to compare the associated decl
        if (tn->getAsTemplateDecl() == nullptr)
          return false;    // be conservative if we can't compare decls
        for (const TemplateName& tpl_name : tpl_names) {
          if (tpl_name.getAsTemplateDecl() == tn->getAsTemplateDecl())
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
        return ContainsKey(others, node.GetAs<void>());
      }
    }

    void AddAll(const NodeSet& that) {
      Extend(&typelocs, that.typelocs);
      Extend(&nnslocs, that.nnslocs);
      Extend(&tpl_names, that.tpl_names);
      Extend(&tpl_args, that.tpl_args);
      Extend(&tpl_arglocs, that.tpl_arglocs);
      InsertAllInto(that.others, &others);
    }

    // Needed since we're treated like an stl-like object.
    bool empty() const {
      return (typelocs.empty() && nnslocs.empty() &&
              tpl_names.empty() && tpl_args.empty() &&
              tpl_arglocs.empty() && others.empty());
    }
    void clear() {
      typelocs.clear();
      nnslocs.clear();
      tpl_names.clear();
      tpl_args.clear();
      tpl_arglocs.clear();
      others.clear();
    }

   private:
    friend class AstFlattenerVisitor;

    // It's ok not to check for duplicates; we're just traversing the tree.
    void Add(TypeLoc tl) { typelocs.push_back(tl); }
    void Add(NestedNameSpecifierLoc nl) { nnslocs.push_back(nl); }
    void Add(TemplateName tn) { tpl_names.push_back(tn); }
    void Add(TemplateArgument ta) { tpl_args.push_back(ta); }
    void Add(TemplateArgumentLoc tal) { tpl_arglocs.push_back(tal); }
    void Add(const void* o) { others.insert(o); }

    vector<TypeLoc> typelocs;
    vector<NestedNameSpecifierLoc> nnslocs;
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
      TraverseDecl(decl);
      swap(*node_set, seen_nodes_);  // move the seen_nodes_ into the cache
    }
    return *node_set;                // returns the cache entry
  }

  //------------------------------------------------------------
  // Pure virtual methods that the base class requires.

  bool CanIgnoreCurrentASTNode() const override {
    return false;
  }

  bool ShouldPrintSymbolFromCurrentFile() const override {
    return ShouldPrint(7);
  }

  string GetSymbolAnnotation() const override {
    return " in uninstantiated tpl";
  }

  //------------------------------------------------------------
  // Top-level handlers that construct the tree.

  bool VisitDecl(Decl*) {
    AddCurrentAstNodeAsPointer();
    return true;
  }

  bool VisitStmt(Stmt*) {
    AddCurrentAstNodeAsPointer();
    return true;
  }

  bool VisitType(Type*) {
    AddCurrentAstNodeAsPointer();
    return true;
  }

  bool VisitTypeLoc(TypeLoc typeloc) {
    seen_nodes_.Add(typeloc);
    return true;
  }

  bool VisitNestedNameSpecifier(NestedNameSpecifier*) {
    AddCurrentAstNodeAsPointer();
    return true;
  }

  bool VisitTemplateName(TemplateName tpl_name) {
    seen_nodes_.Add(tpl_name);
    return true;
  }

  bool VisitTemplateArgument(const TemplateArgument& tpl_arg) {
    seen_nodes_.Add(tpl_arg);
    return true;
  }

  bool VisitTemplateArgumentLoc(const TemplateArgumentLoc& tpl_argloc) {
    seen_nodes_.Add(tpl_argloc);
    return true;
  }

  bool TraverseImplicitDestructorCall(CXXDestructorDecl* decl,
                                      const Type* type) {
    AddAstNodeAsPointer(decl);
    return Base::TraverseImplicitDestructorCall(decl, type);
  }

  bool HandleFunctionCall(FunctionDecl* callee, const Type* parent_type,
                          const Expr* calling_expr) {
    AddAstNodeAsPointer(callee);
    return Base::HandleFunctionCall(callee, parent_type, calling_expr);
  }

  //------------------------------------------------------------
  // Class logic.

  void AddAstNodeAsPointer(const void* node) {
    seen_nodes_.Add(node);
  }

  void AddCurrentAstNodeAsPointer() {
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
// --- VisitorState
// ----------------------------------------------------------------------
//
// This is a simple struct holding data that IwyuBaseASTVisitor will
// need to access and manipulate.  It's held separately from
// IwyuBaseASTVisitor because we want this information to be shared
// between the IwyuASTConsumer and the InstantiatedTemplateVisitor,
// each of which gets its own copy of IwyuBaseASTVisitor data.  So to
// share data, we need to hold it somewhere else.

struct VisitorState {
  VisitorState(CompilerInstance* c, const IwyuPreprocessorInfo& ipi)
      : compiler(c), preprocessor_info(ipi) {}

  CompilerInstance* const compiler;

  // Information gathered at preprocessor time, including #include info.
  const IwyuPreprocessorInfo& preprocessor_info;

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
  set<SourceLocation> processed_overload_locs;
};

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

  explicit IwyuBaseAstVisitor(VisitorState* visitor_state)
      : Base(visitor_state->compiler),
        visitor_state_(visitor_state) {}

  ~IwyuBaseAstVisitor() override = default;

  // To avoid having this-> pointers everywhere, we re-export Base's
  // functions that we use in this class.  This is a language nit(?)
  // when a templated class subclasses from another templated class.
  using Base::CanIgnoreCurrentASTNode;
  using Base::CurrentLoc;
  using Base::CurrentFileEntry;
  using Base::PrintableCurrentLoc;
  using Base::current_ast_node;
  using Base::compiler;

  enum class IgnoreKind {
    ForUse,
    ForExpansion,
  };

  //------------------------------------------------------------
  // Pure virtual methods that a subclass must implement.

  // Returns true if we are not interested in iwyu information for the
  // given type, where the type is *not* the current AST node.
  // TODO(csilvers): check we're calling this consistent with its API.
  virtual bool CanIgnoreType(const Type* type,
                             IgnoreKind = IgnoreKind::ForUse) const = 0;

  // Returns true if we are not interested in doing an iwyu check on
  // the given decl, where the decl is *not* the current AST node.
  // TODO(csilvers): check we're calling this consistent with its API.
  virtual bool CanIgnoreDecl(const Decl* decl) const = 0;

  //------------------------------------------------------------
  // IWYU logic.

  // Helper for MapPrivateDeclToPublicDecl.  Returns true if the decl
  // is a template specialization whose (written qualified) name matches
  // the given name, has the given number of template arguments, and
  // whose specified tpl argument is a type.
  bool DeclIsTemplateWithNameAndNumArgsAndTypeArg(
      const Decl* decl, const string& name,
      size_t num_args, size_t type_arg_idx) const {
    const ClassTemplateSpecializationDecl* tpl_decl = DynCastFrom(decl);
    if (!tpl_decl)
      return false;
    const string actual_name = GetWrittenQualifiedNameAsString(tpl_decl);
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

  // Some types, such as __gnu_cxx::__normal_iterator or std::__wrap_iter, are
  // private types that should not be exposed to the user.  Instead, they're
  // exposed to the user via typedefs, like vector::iterator.
  // Sometimes, the typedef gets lost (such as for find(myvec.begin(),
  // myvec.end(), foo)), so we need to manually map back.  We map
  // __normal_iterator<foo, vector> to vector<> and __wrap_iter<foo> to foo,
  // assuming that the vector<> class includes the typedef.  Likewise, we map
  // any free function taking a private iterator (such as operator==) the
  // same way, assuming that that (templatized) function is instantiated
  // as part of the vector class.
  //    We do something similar for _List_iterator and _List_const_iterator
  // from GNU libstdc++, and for __list_iterator and __list_const_iterator
  // from libc++.  These private names are defined in stl_list.h and list
  // respectively, so we don't need to re-map them, but we do want to re-map
  // reverse_iterator<_List_iterator> to something in list header.
  //    If the input decl does not correspond to one of these private
  // decls, we return nullptr.  This method is actually a helper for
  // MapPrivateDeclToPublicDecl() and MapPrivateTypeToPublicType().
  const Type* MapPrivateDeclToPublicType(const NamedDecl* decl) const {
    const NamedDecl* class_decl = decl;
    // If we're a member method, then the __normal_iterator or __wrap_iter will
    // be the parent: __normal_iterator::operator=.  If we're a free
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

    // In addition to __normal_iterator<x> and __wrap_iter<x>, we want
    // to handle reverse_iterator<__normal_iterator<x>>, and in the same way.
    if (DeclIsTemplateWithNameAndNumArgsAndTypeArg(
            class_decl, "std::reverse_iterator", 1, 0)) {
      const Type* reversed_iterator_type = GetTplTypeArg(class_decl, 0);
      // Gets class_decl to be reversed iterator.
      class_decl = TypeToDeclAsWritten(reversed_iterator_type);

      // If it's reverse_iterator<_List_iterator>, map to
      // _List_iterator, which is defined in stl_list like we want.  Also map
      // reverse_iterator<__list_iterator> to __list_iterator which is defined
      // in list.
      if (DeclIsTemplateWithNameAndNumArgsAndTypeArg(
              class_decl, "std::_List_iterator", 1, 0) ||
          DeclIsTemplateWithNameAndNumArgsAndTypeArg(
              class_decl, "std::_List_const_iterator", 1, 0) ||
          DeclIsTemplateWithNameAndNumArgsAndTypeArg(
              class_decl, "std::__list_iterator", 2, 0) ||
          DeclIsTemplateWithNameAndNumArgsAndTypeArg(
              class_decl, "std::__list_const_iterator", 2, 0)) {
        return reversed_iterator_type;
      }
    }

    if (DeclIsTemplateWithNameAndNumArgsAndTypeArg(
            class_decl, "__gnu_cxx::__normal_iterator", 2, 1)) {
      return GetTplTypeArg(class_decl, 1);
    }
    if (DeclIsTemplateWithNameAndNumArgsAndTypeArg(
            class_decl, "std::__wrap_iter", 1, 0)) {
      return GetTplTypeArg(class_decl, 0);
    }

    return nullptr;
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

  // Get the canonical use location for a (location, decl) pair.
  // Decide whether the file expanding the macro or the file defining the macro
  // should be held responsible for a use.
  SourceLocation GetCanonicalUseLocation(SourceLocation use_loc,
                                         const NamedDecl* decl) {
    CHECK_(decl != nullptr) << ": Need a decl to compute use location";

    // If we're not in a macro, just echo the use location.
    if (!use_loc.isMacroID())
      return use_loc;

    VERRS(5) << "Trying to determine use location for '" << PrintableDecl(decl)
             << "'\n";

    SourceManager* sm = GlobalSourceManager();
    SourceLocation spelling_loc = sm->getSpellingLoc(use_loc);
    SourceLocation expansion_loc = sm->getExpansionLoc(use_loc);

    // If the file defining the macro contains a forward decl, keep it around
    // and treat it as a hint that the expansion loc is responsible for the
    // symbol.
    OptionalFileEntryRef macro_def_file = GetLocFileEntry(spelling_loc);
    VERRS(5) << "Macro is defined in '" << GetFilePath(macro_def_file) << "'\n";

    const NamedDecl* fwd_decl = nullptr;
    for (const NamedDecl* redecl : GetTagRedecls(decl)) {
      if (GetFileEntry(redecl) == macro_def_file && IsForwardDecl(redecl)) {
        VERRS(5) << "Found fwd-decl hint at "
                 << PrintableLoc(GetLocation(redecl)) << "\n";
        fwd_decl = redecl;
        break;
      }
    }

    if (!fwd_decl) {
      if (const auto* func_decl = dyn_cast<FunctionDecl>(decl)) {
        if (const FunctionTemplateDecl* ft_decl =
                func_decl->getPrimaryTemplate()) {
          VERRS(5) << "No fwd-decl found, looking for function template decl\n";
          for (const NamedDecl* redecl : ft_decl->redecls()) {
            if (GetFileEntry(redecl) == macro_def_file) {
              VERRS(5) << "Found function template at "
                       << PrintableLoc(GetLocation(redecl)) << "\n";
              fwd_decl = redecl;
              break;
            }
          }
        }
      }
    }

    if (fwd_decl) {
      // Make sure we keep that forward-declaration, even if it's probably
      // unused in this file.
      IwyuFileInfo* file_info = preprocessor_info().FileInfoFor(macro_def_file);
      file_info->ReportForwardDeclareUse(
          spelling_loc, fwd_decl, ComputeUseFlags(current_ast_node()), nullptr);
    }

    // Resolve the best use location based on our current knowledge.
    //
    // 1) If the use_loc is in <scratch space>, we assume it's formed by macro
    //    argument concatenation, and attribute responsibility to the expansion
    //    location.
    // 2) If the macro definition file forward-declares the used decl, that's a
    //    hint that it wants the expansion location to take responsibility.
    //
    // Otherwise, the spelling loc is responsible.
    const char* side;
    if (IsInScratchSpace(spelling_loc)) {
      VERRS(5) << "Spelling location is in <scratch space>, presumably as a "
               << "result of macro arg concatenation\n";
      use_loc = expansion_loc;
      side = "expansion";
    } else if (fwd_decl != nullptr) {
      VERRS(5) << "Found a hint decl in macro definition file\n";
      use_loc = expansion_loc;
      side = "expansion";
    } else {
      use_loc = spelling_loc;
      side = "spelling";
    }

    VERRS(4) << "Attributing use of '" << PrintableDecl(decl) << "' to " << side
             << " location at " << PrintableLoc(use_loc) << "\n";

    return use_loc;
  }

  // There are a few situations where iwyu is more restrictive than
  // C++: where C++ allows a forward-declare but iwyu wants the full
  // type.  One is in a typedef: if you write 'typedef Foo MyTypedef',
  // iwyu says that you are responsible for #including "foo.h", but
  // the language allows a forward-declare.  Another is for
  // 'autocast': if your function has a parameter with a conversion
  // (one-arg, not-explicit) constructor, iwyu requires that the
  // function-author provides the full type of that parameter, but
  // the language doesn't.  (It's ok with all callers providing the
  // full type instead.)
  //
  // In each of these situations, we allow the user to say that iwyu
  // should not require #includes for these underlying types, but
  // allow forward-declares instead.  The author can do this by
  // explicitly forward-declaring in the same file: for instance, they
  // would do
  //    class Foo; typedef Foo MyTypedef;   // can be on different lines :-)
  //    class AutocastType; void MyFunc(AutocastType);   // but in same file
  // If a definition- or declaration-site does this forward-declaring
  // *and* does not directly #include the necessary file for Foo or
  // AutocastType, we take that as a signal from the code-author that
  // iwyu should relax its policies.  These functions calculate the
  // types (which may have many component-types if it's a templated
  // type) for which the code-author has made this decision.
  bool CodeAuthorWantsJustAForwardDeclare(const Type* type,
                                          SourceLocation use_loc) const {
    const NamedDecl* decl = TypeToDeclAsWritten(type);
    if (decl == nullptr)   // only class-types are candidates for returning true
      return false;

    // Sometimes a type points back to an implicit decl (e.g. a bultin type),
    // and we can't do author-intent analysis without location information.
    // Assume that it's not forward-declarable.
    if (decl->isImplicit()) {
      VERRS(5) << "Skipping forward-declare analysis for implicit decl: '"
               << PrintableDecl(decl) << "'\n";
      return false;
    }

    // If we're a template specialization, we also accept
    // forward-declarations of the underlying template (vector<T>, not
    // vector<int>).
    set<const NamedDecl*> redecls = GetTagRedecls(decl);
    if (const ClassTemplateSpecializationDecl* spec_decl = DynCastFrom(decl)) {
      InsertAllInto(GetTagRedecls(spec_decl->getSpecializedTemplate()),
                    &redecls);
    }

    // Check if the author forward-declared the class in the same file.
    bool found_earlier_forward_declare_in_same_file = false;
    for (const NamedDecl* redecl : redecls) {
      if (IsBeforeInSameFile(redecl, use_loc)) {
        found_earlier_forward_declare_in_same_file = true;
        break;
      }
    }
    if (!found_earlier_forward_declare_in_same_file)
      return false;

    // Check if the the author is not #including the file with the
    // definition.  PublicHeaderIntendsToProvide has exactly the
    // semantics we want.  Note if there's no definition anywhere, we
    // say the author does not want the full type (which is a good
    // thing, since there isn't one!)
    if (const NamedDecl* dfn = GetTagDefinition(decl)) {
      if (IsBeforeInSameFile(dfn, use_loc))
        return false;
      if (preprocessor_info().PublicHeaderIntendsToProvide(
              GetFileEntry(use_loc), GetFileEntry(dfn))) {
        return false;
      }
    }

    // OK, looks like the author has stated they don't want the fulll type.
    return true;
  }

  set<const Type*> GetProvidedTypesForAlias(const Type* underlying_type,
                                            SourceLocation decl_loc) const {
    return GetProvidedTypes(underlying_type, decl_loc);
  }

  // ast_node is the node for the autocast CastExpr.  We use it to get
  // the parent CallExpr to figure out what function is being called.
  set<const Type*> GetProvidedTypesForAutocast(const ASTNode* ast_node) const {
    while (ast_node && !ast_node->IsA<CallExpr>())
      ast_node = ast_node->parent();
    CHECK_(ast_node && "Should only check Autocast if under a CallExpr");
    const CallExpr* call_expr = ast_node->GetAs<CallExpr>();
    const FunctionDecl* fn_decl = call_expr->getDirectCallee();
    if (!fn_decl)     // TODO(csilvers): what to do for fn ptrs and the like?
      return set<const Type*>();

    // Collect the non-explicit, one-arg constructor ('autocast') types.
    set<const Type*> autocast_types;
    for (FunctionDecl::param_const_iterator param = fn_decl->param_begin();
         param != fn_decl->param_end(); ++param) {
      const Type* param_type = GetTypeOf(*param);
      if (HasImplicitConversionConstructor(param_type)) {
        const Type* deref_param_type =
            RemovePointersAndReferencesAsWritten(param_type);
        autocast_types.insert(Desugar(deref_param_type));
      }
    }

    // Now look at all the function decls that are visible from the
    // call-location.  A type component is considered as provided if *any*
    // of the function decls provides it.
    set<const Type*> retval;
    for (FunctionDecl::redecl_iterator fn_redecl = fn_decl->redecls_begin();
         fn_redecl != fn_decl->redecls_end(); ++fn_redecl) {
      // Ignore function-decls that we can't see from the use-location.
      if (!preprocessor_info().FileTransitivelyIncludes(
              GetFileEntry(call_expr), GetFileEntry(*fn_redecl))) {
        continue;
      }
      if (fn_redecl->isThisDeclarationADefinition() && !IsInHeader(*fn_redecl))
        continue;
      for (const Type* type : autocast_types) {
        const set<const Type*> components =
            GetProvidedTypes(type, GetLocation(*fn_redecl));
        retval.insert(components.begin(), components.end());
      }
    }

    return retval;
  }

  set<const Type*> GetProvidedTypesForFnReturn(const FunctionDecl* decl) const {
    const Type* return_type = decl->getReturnType().getTypePtr();
    return GetProvidedTypes(return_type, GetLocation(decl));
  }

  //------------------------------------------------------------
  // Checkers, that tell iwyu_output about uses of symbols.
  // We let, but don't require, subclasses to override these.

  // The comment, if not nullptr, is extra text that is included along with
  // the warning message that iwyu emits. The extra use flags is optional
  // info that can be assigned to the use (see the UF_* constants)
  virtual void ReportDeclUse(SourceLocation used_loc,
                             const NamedDecl* used_decl,
                             const char* comment = nullptr,
                             UseFlags extra_use_flags = 0) {
    const NamedDecl* target_decl = used_decl;

    // Sometimes a shadow decl comes between us and the 'real' decl.
    const UsingDecl* using_decl = nullptr;
    if (const auto* shadow_decl = dyn_cast<UsingShadowDecl>(used_decl)) {
      target_decl = shadow_decl->getTargetDecl();
      using_decl = dyn_cast<UsingDecl>(shadow_decl->getIntroducer());
    }

    // Map private decls like __normal_iterator to their public counterpart.
    target_decl = MapPrivateDeclToPublicDecl(target_decl);
    if (CanIgnoreDecl(target_decl))
      return;

    UseFlags use_flags = ComputeUseFlags(current_ast_node()) | extra_use_flags;

    // Canonicalize the use location and report the use.
    used_loc = GetCanonicalUseLocation(used_loc, target_decl);
    OptionalFileEntryRef used_in = GetFileEntry(used_loc);

    preprocessor_info().FileInfoFor(used_in)->ReportFullSymbolUse(
        used_loc, target_decl, use_flags, comment);

    // Sometimes using a decl drags in a few other uses as well:

    // If we're a use that depends on a using declaration, make sure
    // we #include the file with the using declaration. Need to check
    // the original reported decl so we don't lose the shadow information.
    // TODO(csilvers): check that our getQualifier() does not match
    // the namespace of the decl.  If we have 'using std::vector;' +
    // 'std::vector<int> foo;' we don't actually care about the
    // using-decl.
    // TODO(csilvers): maybe just insert our own using declaration
    // instead?  We can call it "Use what you use". :-)
    // TODO(csilvers): check for using directives too.
    if (using_decl) {
      preprocessor_info().FileInfoFor(used_in)->ReportUsingDeclUse(
          used_loc, using_decl, use_flags, "(for using decl)");
    }
  }

  // The comment, if not nullptr, is extra text that is included along
  // with the warning message that iwyu emits.
  virtual void ReportDeclForwardDeclareUse(SourceLocation used_loc,
                                           const NamedDecl* used_decl,
                                           const char* comment = nullptr) {
    const NamedDecl* target_decl = used_decl;

    // Sometimes a shadow decl comes between us and the 'real' decl.
    const UsingDecl* using_decl = nullptr;
    if (const auto* shadow_decl = dyn_cast<UsingShadowDecl>(used_decl)) {
      target_decl = shadow_decl->getTargetDecl();
      using_decl = dyn_cast<UsingDecl>(shadow_decl->getIntroducer());
    }

    target_decl = MapPrivateDeclToPublicDecl(target_decl);
    if (CanIgnoreDecl(target_decl))
      return;

    UseFlags use_flags = ComputeUseFlags(current_ast_node());

    // Canonicalize the use location and report the use.
    used_loc = GetCanonicalUseLocation(used_loc, target_decl);
    OptionalFileEntryRef used_in = GetFileEntry(used_loc);
    preprocessor_info().FileInfoFor(used_in)->ReportForwardDeclareUse(
        used_loc, target_decl, use_flags, comment);

    // If we're a use that depends on a using declaration, make sure
    // we #include the file with the using declaration.
    if (using_decl) {
      preprocessor_info().FileInfoFor(used_in)->ReportUsingDeclUse(
          used_loc, using_decl, use_flags, "(for using decl)");
    }
  }

  void ReportDeclsUse(SourceLocation used_loc,
                      const set<const NamedDecl*>& decls) {
    for (const NamedDecl* decl : decls)
      ReportDeclUse(used_loc, decl);
  }

  // Called when the given type is fully used at used_loc, regardless
  // of the type being explicitly written in the source code or not.
  virtual void ReportTypeUse(SourceLocation used_loc, const Type* type,
                             DerefKind deref_kind) {
    ReportTypeUseInternal(used_loc, type, blocked_types_, deref_kind);
  }

  void ReportTypesUse(SourceLocation used_loc, const set<const Type*>& types) {
    for (const Type* type : types)
      ReportTypeUse(used_loc, type, DerefKind::None);
  }

  //------------------------------------------------------------
  // Visitors of types derived from Decl.

  // Friend declarations only need their types forward-declared.
  bool VisitFriendDecl(FriendDecl* decl) {
    if (CanIgnoreCurrentASTNode())
      return true;
    current_ast_node()->set_in_forward_declare_context(true);
    return true;
  }

  bool VisitFriendTemplateDecl(FriendTemplateDecl* decl) {
    if (CanIgnoreCurrentASTNode())
      return true;
    current_ast_node()->set_in_forward_declare_context(true);
    return true;
  }

  // If you say 'typedef Foo Bar', C++ says you just need to
  // forward-declare Foo.  But iwyu would rather you fully define Foo,
  // so all users of Bar don't have to.  We make two exceptions:
  // 1) The author of the typedef doesn't *want* to provide Foo, and
  //    is happy making all the callers do so.  The author indicates
  //    this by explicitly forward-declaring Foo and not #including
  //    foo.h.
  // 2) The typedef is a member of a templated class, and the
  //    underlying type is a template parameter:
  //       template<class T> struct C { typedef T value_type; };
  //    This is not a re-export because you need the type to
  //    access the typedef (via 'C<Someclass>::value_type'), so
  //    there's no need for the typedef-file to provide the type
  //    too.  TODO(csilvers): this is patently wrong; figure out
  //    something better.  We need something that doesn't require
  //    the full type info for creating a scoped_ptr<MyClass>.
  // As an extension of (2), if the typedef is a template type that
  // contains T as a template parameter, the typedef still re-exports
  // the template type (it's not (2)), but the template parameter
  // itself can be forward-declared, just as in (2).  That is:
  //   template<class T> struct C { typedef pair<T,T> value_type; };
  // iwyu will demand the full type of pair, but not of its template
  // arguments.  This is handled in VisitSubstTemplateTypeParmType.
  // Typedef underlying type is marked fwd-declarable by default, and the
  // aforementioned analysis is performed with 'IsProvidedTypeComponent'
  // function.
  bool VisitTypedefNameDecl(TypedefNameDecl* decl) {
    if (CanIgnoreCurrentASTNode())
      return true;
    current_ast_node()->set_in_forward_declare_context(true);
    return true;
  }

  // Default type template arguments are forward-declarable, but can be provided
  // by the template author similarly to 'typedef' underlying types. They are
  // handled by 'IsProvidedTypeComponent'.
  bool VisitTemplateTypeParmDecl(TemplateTypeParmDecl*) {
    if (CanIgnoreCurrentASTNode())
      return true;
    current_ast_node()->set_in_forward_declare_context(true);
    return true;
  }

  // If we're a declared (not defined) function, all our types --
  // return type and argument types -- are forward-declarable.  The
  // one exception required by the language is the throw types, which
  // we clean up in VisitType().
  // There are two more exceptions that iwyu imposes:
  // (1) iwyu asks the function author to provide the full type
  //     information for the return type.  That way the user doesn't
  //     have to.
  // (2) If any of our function parameters have a type with a
  //     non-explicit, one-arg constructor, or is a const reference to
  //     such a type, mark that type as not forward declarable.  The
  //     worry is that people might need the full type for the
  //     implicit conversion (the 'autocast'), for instance, passing
  //     in a char* to Fn(const StringPiece& foo) { ... }
  // Both of these iwyu requirements can be overridden by the function
  // author; for details, see CodeAuthorWantsJustAForwardDeclare.
  bool VisitFunctionDecl(FunctionDecl* decl) {
    if (CanIgnoreCurrentASTNode())
      return true;

    if (decl->isThisDeclarationADefinition()) {
      // For free functions, report use of all previously seen decls.
      if (decl->getKind() == Decl::Function) {
        FunctionDecl* redecl = decl;
        while ((redecl = redecl->getPreviousDecl()))
          ReportDeclUse(CurrentLoc(), redecl);
      }
      if (!IsInHeader(decl)) {
        // No point in author-intent analysis of function definitions
        // in source files or for builtins.
        return true;
      }
    } else {
      // Make all our types forward-declarable.
      current_ast_node()->set_in_forward_declare_context(true);
    }
    return true;
  }

  // Special handling for C++ methods to detect covariant return types.
  // These are defined as a derived class overriding a method with a different
  // return type from the base.
  bool VisitCXXMethodDecl(CXXMethodDecl* method_decl) {
    if (CanIgnoreCurrentASTNode())
      return true;

    if (HasCovariantReturnType(method_decl)) {
      VERRS(6) << "Found covariant return type in "
               << method_decl->getQualifiedNameAsString()
               << ", needs complete type\n";

      ReportTypeUse(CurrentLoc(), method_decl->getReturnType().getTypePtr(),
                    DerefKind::RemoveRefsAndPtr);
    }

    return true;
  }

  //------------------------------------------------------------
  // Visitors of types derived from Stmt.

  // Catch statements always require the full type to be visible,
  // no matter if we're catching by value, reference or pointer.
  bool VisitCXXCatchStmt(CXXCatchStmt* stmt) {
    if (CanIgnoreCurrentASTNode())
      return true;

    if (const VarDecl* exception_decl = stmt->getExceptionDecl()) {
      // Get the caught type from the decl via associated type source info to
      // get more precise location info for the type use.
      TypeLoc typeloc = exception_decl->getTypeSourceInfo()->getTypeLoc();
      const Type* caught_type = typeloc.getType().getTypePtr();

      // Strip off pointers/references to get to the pointee type.
      ReportTypeUse(GetLocation(&typeloc), caught_type,
                    DerefKind::RemoveRefsAndPtr);
    } else {
      // catch(...): no type to act on here.
    }

    return true;
  }

  // The explicit traversal of 'CXXForRangeStmt' is introduced to avoid
  // a problem with code like this:
  //
  // using ProvidingRefAlias = Container&;
  // ProvidingRefAlias ref_provided = ...;
  // for (int x : ref_provided) ...
  //
  // After initialization of implicit range variable, the specific providing
  // type is desugared. Due to this, it is reported e.g. on handling
  // the implicit call of 'begin()' method despite being provided by alias.
  // Explicit traversal without handling for-loop implicit variables allows
  // to avoid such an unwanted reporting of the provided type. The code does
  // essentially the same that the default implementation does when
  // 'shouldVisitImplicitCode()' returns 'false'.
  bool TraverseCXXForRangeStmt(CXXForRangeStmt* stmt) {
    if (!this->getDerived().WalkUpFromCXXForRangeStmt(stmt))
      return false;
    if (Stmt* init_stmt = stmt->getInit()) {
      if (!this->getDerived().TraverseStmt(init_stmt))
        return false;
    }
    if (!this->getDerived().TraverseStmt(stmt->getLoopVarStmt()))
      return false;
    if (!this->getDerived().TraverseStmt(stmt->getRangeInit()))
      return false;
    return this->getDerived().TraverseStmt(stmt->getBody());
  }

  // The type of the for-range-init expression is fully required, because the
  // compiler generates method calls to it, e.g. 'for (auto t : ts)' translates
  // roughly into 'for (auto i = std::begin(ts); i != std::end(ts); ++i)'.
  // Both the iterator type and the begin/end calls depend on the complete type
  // of ts, so make sure we include it.
  bool VisitCXXForRangeStmt(CXXForRangeStmt* stmt) {
    if (CanIgnoreCurrentASTNode())
      return true;

    if (const Type* type = GetTypeOf(stmt->getRangeInit())) {
      ReportTypeUse(CurrentLoc(), type, DerefKind::RemoveRefs);

      // TODO: We should probably find a way to require inclusion of any
      // argument-dependent begin/end declarations.
    }

    return true;
  }

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
  bool VisitCastExpr(CastExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;
    const Type* from_type = GetTypeOf(expr->getSubExprAsWritten());
    const Type* to_type = GetTypeOf(expr);

    // If this cast requires a user-defined conversion of the from-type, look up
    // its return type so we can see through up/down-casts via such conversions.
    const Type* converted_from_type = nullptr;
    if (const NamedDecl* conv_decl = expr->getConversionFunction()) {
      converted_from_type =
          cast<FunctionDecl>(conv_decl)->getReturnType().getTypePtr();
    }

    std::vector<const Type*> required_full_types;

    // The list of kinds: http://clang.llvm.org/doxygen/namespaceclang.html
    switch (expr->getCastKind()) {
      // This cast still isn't handled directly.
      case clang::CK_Dependent:
        break;

      // These casts don't require any iwyu action.
      case clang::CK_LValueToRValue:
      case clang::CK_AtomicToNonAtomic:
      case clang::CK_NonAtomicToAtomic:
      case clang::CK_ReinterpretMemberPointer:
      case clang::CK_BuiltinFnToFnPtr:
      case clang::CK_ZeroToOCLOpaqueType: // OpenCL opaque types are built-in.
      case clang::CK_IntToOCLSampler: // OpenCL sampler_t is a built-in type.
      case clang::CK_AddressSpaceConversion:  // Address spaces are associated
                                              // with pointers, so no need for
                                              // the full type.
        break;

      // Ignore non-ptr-to-ptr casts.
      case clang::CK_ArrayToPointerDecay:
      case clang::CK_BooleanToSignedIntegral:
      case clang::CK_FixedPointCast:
      case clang::CK_FixedPointToBoolean:
      case clang::CK_FixedPointToFloating:
      case clang::CK_FixedPointToIntegral:
      case clang::CK_FloatingCast:
      case clang::CK_FloatingComplexCast:
      case clang::CK_FloatingComplexToBoolean:
      case clang::CK_FloatingComplexToIntegralComplex:
      case clang::CK_FloatingComplexToReal:
      case clang::CK_FloatingRealToComplex:
      case clang::CK_FloatingToBoolean:
      case clang::CK_FloatingToFixedPoint:
      case clang::CK_FloatingToIntegral:
      case clang::CK_FunctionToPointerDecay:
      case clang::CK_IntegralCast:
      case clang::CK_IntegralComplexCast:
      case clang::CK_IntegralComplexToBoolean:
      case clang::CK_IntegralComplexToFloatingComplex:
      case clang::CK_IntegralComplexToReal:
      case clang::CK_IntegralRealToComplex:
      case clang::CK_IntegralToBoolean:
      case clang::CK_IntegralToFixedPoint:
      case clang::CK_IntegralToFloating:
      case clang::CK_IntegralToPointer:
      case clang::CK_MatrixCast:
      case clang::CK_MemberPointerToBoolean:
      case clang::CK_NullToMemberPointer:
      case clang::CK_NullToPointer:
      case clang::CK_PointerToBoolean:
      case clang::CK_PointerToIntegral:
      case clang::CK_ToUnion:
      case clang::CK_ToVoid:
        break;

      case clang::CK_AnyPointerToBlockPointerCast:
      case clang::CK_ARCConsumeObject:
      case clang::CK_ARCExtendBlockObject:
      case clang::CK_ARCProduceObject:
      case clang::CK_ARCReclaimReturnedObject:
      case clang::CK_BlockPointerToObjCPointerCast:
      case clang::CK_CopyAndAutoreleaseBlockObject:
      case clang::CK_CPointerToObjCPointerCast:
      case clang::CK_ObjCObjectLValueCast:
      case clang::CK_VectorSplat:
        CHECK_UNREACHABLE_(
            "TODO(csilvers): for objc and clang lang extensions");
        break;

      // Kinds for reinterpret_cast and const_cast, which need no full types.
      case clang::CK_BitCast:                // used for reinterpret_cast
      case clang::CK_LValueBitCast:          // used for reinterpret_cast
      case clang::CK_LValueToRValueBitCast:  // used for reinterpret_cast
      case clang::CK_NoOp:                   // used for const_cast, etc
        break;

      // Need the full to-type so we can call its constructor.
      case clang::CK_ConstructorConversion:
        // 'Autocast' in function calls is handled in VisitCXXConstructExpr.
        if (!current_ast_node()->template HasAncestorOfType<CallExpr>())
          required_full_types.push_back(to_type);
        break;
      // Need the full from-type so we can call its 'operator <totype>()'.
      case clang::CK_UserDefinedConversion:
        required_full_types.push_back(from_type);
        break;

      // Kinds that cast up or down an inheritance hierarchy.
      case clang::CK_BaseToDerived:
      case clang::CK_BaseToDerivedMemberPointer:
        // Just 'to' type is enough: full type for derived gets base type too.
        required_full_types.push_back(to_type);
        break;
      case clang::CK_DerivedToBase:
      case clang::CK_UncheckedDerivedToBase:
      case clang::CK_DerivedToBaseMemberPointer:
        // Just 'from' type is enough: full type for derived gets base type too.
        required_full_types.push_back(from_type);

        // If this derived-to-base cast had an associated conversion function,
        // it's a user-defined conversion operator. The from-type in this cast
        // is not necessarily related to the base type, but the converted
        // from-type must be, so make sure we require both.
        if (converted_from_type) {
          required_full_types.push_back(converted_from_type);
        }
        break;
      case clang::CK_Dynamic:
        // Usually dynamic casting is a base-to-derived cast, but it is
        // possible to dynamic-cast between siblings, in which case we
        // need both types.
        required_full_types.push_back(from_type);
        required_full_types.push_back(to_type);
        break;
    }

    for (const Type* type : required_full_types)
      ReportTypeUse(CurrentLoc(), type, DerefKind::RemoveRefsAndPtr);

    return true;
  }

  // Mark that we need the full type info for our base type -- the
  // thing we're a member of -- and it's not just forward-declarable.
  // For instance, for code 'Mytype* myvar; myvar->a;', we'll get a
  // MemberExpr callback whose base has the type of myvar.
  bool VisitMemberExpr(MemberExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;

    const Expr* base_expr = expr->getBase()->IgnoreParenImpCasts();
    const Type* base_type = GetTypeOf(base_expr);
    CHECK_(base_type && "Member's base does not have a type?");
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
    ReportTypeUse(CurrentLoc(), base_type, DerefKind::RemoveRefsAndPtr);
    return true;
  }

  // For a[4], report that we need the full type of *a (to get its
  // size; otherwise the compiler can't tell the address of a[4]).
  bool VisitArraySubscriptExpr(ArraySubscriptExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;

    ReportTypeUse(CurrentLoc(), GetTypeOf(expr->getLHS()),
                  DerefKind::RemoveRefsAndPtr);
    return true;
  }

  // If a binary operator expression results in pointer arithmetic, we need the
  // full types of all pointers involved.
  bool VisitBinaryOperator(BinaryOperator* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;

    // If it's not +, +=, - or -=, this can't be pointer arithmetic
    BinaryOperator::Opcode op = expr->getOpcode();
    if (op != clang::BO_Add && op != clang::BO_Sub &&
        op != clang::BO_AddAssign && op != clang::BO_SubAssign)
      return true;

    for (const Stmt* child : expr->children()) {
      const Type* type = GetTypeOf(cast<Expr>(child));
      if (type->isPointerType()) {
        // It's a pointer-typed expression.
        ReportTypeUse(CurrentLoc(), type, DerefKind::RemoveRefsAndPtr);
      }
    }

    return true;
  }

  bool VisitTypeTraitExpr(TypeTraitExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;

    // We assume that all type traits with >= 2 arguments, except '__is_same',
    // require full type information even for pointer types. For example,
    // this is the case for `__is_convertible_to` trait.
    if (expr == nullptr || expr->getNumArgs() < 2 ||
        expr->getTrait() == TypeTrait::BTT_IsSame) {
      return true;
    }

    for (const TypeSourceInfo* arg : expr->getArgs()) {
      QualType qual_type = arg->getType();
      const Type* type = qual_type.getTypePtr();
      ReportTypeUse(CurrentLoc(), type, DerefKind::RemoveRefsAndPtr);
    }

    return true;
  }

  // Mark that we need the full type info for the thing we're taking
  // sizeof of.  Sometimes this is double-counting: for
  // sizeof(some_type), RecursiveASTVisitor will visit some_type and
  // say it needs the full type information there, and for
  // sizeof(some_var), we'll report we need full type information when
  // some_var is defined.  But if the arg is a reference, nobody else
  // will say we need full type info but us.
  bool VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;

    current_ast_node()->set_in_forward_declare_context(false);

    // Calling sizeof on a reference-to-X is the same as calling it on X.
    // If sizeof() takes a type, this is easy to check.  If sizeof()
    // takes an expr, it's hard to tell -- GetTypeOf(expr) 'sees through'
    // references.  Luckily, we want to see through references, so we
    // just use the GetTypeOf().
    if (expr->isArgumentType()) {
      const TypeLoc& arg_tl = expr->getArgumentTypeInfo()->getTypeLoc();
      const Type* type = arg_tl.getTypePtr();
      if (type->isReferenceType()) {
        ReportTypeUse(GetLocation(&arg_tl), type, DerefKind::RemoveRefs);
      } else {
        // No need to report on non-ref types, RecursiveASTVisitor will get 'em.
      }
    } else {
      const Expr* arg_expr = expr->getArgumentExpr()->IgnoreParenImpCasts();
      // This reports even if the expr ends up not being a reference, but
      // that's ok (if potentially redundant).
      ReportTypeUse(GetLocation(arg_expr), GetTypeOf(arg_expr),
                    DerefKind::RemoveRefs);
    }
    return true;
  }

  // We want to mark use of the base type For 'free function' operator
  // overloads ('ostream& operator<<(ostream& o, int x)') just like we
  // do for member functions ('ostream& ostream::operator<<(int x)')
  // -- for iwyu purposes, 'x << 4' is just semantic sugar around
  // x.operator<<(4).
  bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;

    if (const Expr* owner_expr = GetFirstClassArgument(expr)) {
      const Type* owner_type = GetTypeOf(owner_expr);
      // Note we report the type use is the location of owner_expr
      // (the 'a' in 'a << b' or the 'MACRO' in 'MACRO << b'), rather
      // than our location (which is the '<<').  That way, we properly
      // situate the owner when it's a macro.
      ReportTypeUse(GetLocation(owner_expr), owner_type, DerefKind::RemoveRefs);
    }
    return true;
  }

  // We have to check the type being deleted is fully defined (the
  // language doesn't require it, but bad things happen if it's not:
  // the destructor isn't run).
  bool VisitCXXDeleteExpr(CXXDeleteExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;

    const Expr* delete_arg = expr->getArgument()->IgnoreParenImpCasts();
    // We always delete a pointer, so do one dereference to get the
    // actual type being deleted.
    const Type* delete_ptr_type = GetTypeOf(delete_arg);
    ReportTypeUse(CurrentLoc(), delete_ptr_type, DerefKind::RemoveRefsAndPtr);

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
  void ReportIfReferenceVararg(const Expr* const* args, unsigned num_args,
                               const FunctionProtoType* callee_type) {
    if (callee_type && callee_type->isVariadic()) {
      const unsigned first_vararg_index = callee_type->getNumParams();
      for (unsigned i = first_vararg_index; i < num_args; i++) {
        // We only care about reporting for references, but it's ok if
        // we catch a few non-ref types too (it's just redundant).
        // All expressions that are references will have their
        // valuekind be an LValue, so we use that as the test.
        if (args[i]->getValueKind() == clang::VK_LValue) {
          // The types of expressions 'see through' the reference to
          // the underlying type, which is exactly what we want here.
          ReportTypeUse(CurrentLoc(), GetTypeOf(args[i]),
                        DerefKind::RemoveRefs);
        }
      }
    }
  }
  void ReportIfReferenceVararg(const Expr* const* args, unsigned num_args,
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
  bool VisitCallExpr(CallExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;
    // Nothing to do if the called function is an old K&R-style function.
    const FunctionType* fn_type = GetCalleeFunctionType(expr);
    if (const FunctionProtoType* fn_proto = DynCastFrom(fn_type))
      ReportIfReferenceVararg(expr->getArgs(), expr->getNumArgs(), fn_proto);
    return true;
  }

  bool VisitCXXConstructExpr(CXXConstructExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;
    ReportIfReferenceVararg(expr->getArgs(), expr->getNumArgs(),
                            expr->getConstructor());

    if (IsAutocastExpr(current_ast_node()))
      HandleAutocastOnCallSite(expr);
    else
      HandleNonAutocastConstruction(expr);

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
  bool VisitOverloadExpr(OverloadExpr* expr) {
    // No CanIgnoreCurrentASTNode() check here!  It's later in the function.

    // Make sure all overloads are in the same file.
    if (expr->decls_begin() == expr->decls_end())   // not sure this is possible
      return true;
    const NamedDecl* first_decl = *expr->decls_begin();
    OptionalFileEntryRef first_decl_file_entry = GetFileEntry(first_decl);
    for (OverloadExpr::decls_iterator it = expr->decls_begin();
         it != expr->decls_end(); ++it) {
      if (GetFileEntry(*it) != first_decl_file_entry)
        return true;
    }

    // For now, we're only worried about function calls.
    // TODO(csilvers): are there other kinds of overloads we need to check?
    const FunctionDecl* arbitrary_fn_decl = nullptr;
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
      AddProcessedOverloadLoc(CurrentLoc());
      VERRS(7) << "Adding to processed_overload_locs: "
               << PrintableCurrentLoc() << "\n";
      // Because processed_overload_locs might be set in one visitor
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

  // If getOperatorNew() returns nullptr, it means the operator-new is
  // overloaded, and technically we can't know which operator-new is
  // being called until the template is instantiated.  But if it looks
  // like a placement-new, we handle it at template-writing time
  // anyway.
  bool VisitCXXNewExpr(CXXNewExpr* expr) {
    // Like in VisitOverloadExpr(), we update processed_overload_locs
    // regardless of the value of CanIgnoreCurrentASTNode().

    // We say it's placement-new if it has a single placement-arg, which _might_
    // be a pointer. In templates clang will just say it's a dependent type (or
    // dependent name, for function calls), so we use some heuristics to avoid
    // matching things like 'new (std::nothrow)' and 'new
    // (std::align_val_t(8))'. The pattern 'new (func())' can cause false
    // positives if the eventually looked up 'func()' does not return a pointer
    // type.
    const Expr* placement_expr = nullptr;
    if (!expr->getOperatorNew() && expr->getNumPlacementArgs() == 1) {
      placement_expr = expr->getPlacementArg(0);
    }

    if (placement_expr &&
        (GetTypeOf(placement_expr)->isPointerType() ||
         GetTypeOf(placement_expr)->isArrayType() ||
         IsAddressOf(placement_expr) ||
         IsDependentNameCall(placement_expr))) {
      // Treat this like an OverloadExpr.
      AddProcessedOverloadLoc(CurrentLoc());
      VERRS(7) << "Adding to processed_overload_locs (placement-new): "
               << PrintableCurrentLoc() << "\n";
      if (!CanIgnoreCurrentASTNode()) {
        // We have to 'make up' a full file path for 'new'.  We'll
        // parse it to '<new>' before using, so any path that does
        // that, and is clearly a c++ path, is fine; its exact
        // contents don't matter that much.
        OptionalFileEntryRef use_file = CurrentFileEntry();
        OptionalFileEntryRef file = compiler()->getPreprocessor().LookupFile(
            CurrentLoc(), "new", true, nullptr, RawFileEntry(use_file), nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, false);
        if (file) {
          preprocessor_info().FileInfoFor(use_file)->ReportFullSymbolUse(
              CurrentLoc(), *file, "operator new");
        }
      }
    }

    // We also need to do a varargs check, like for other function calls.
    if (CanIgnoreCurrentASTNode())
      return true;
    // ... only if this NewExpr involves a constructor call.
    const Expr* initializer = expr->getInitializer();
    if (const CXXConstructExpr* cce = DynCastFrom(initializer)) {
      ReportIfReferenceVararg(cce->getArgs(),
                              cce->getNumArgs(),
                              cce->getConstructor());
    }
    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;
    // Report EnumName instead of EnumName::Item.
    // It supports for removing EnumName forward- (opaque-) declarations
    // from output.
    if (const auto* enum_constant_decl =
            dyn_cast<EnumConstantDecl>(expr->getDecl())) {
      const auto* enum_decl =
          cast<EnumDecl>(enum_constant_decl->getDeclContext());

      // For unnamed enums, enumerator name is still preferred.
      if (enum_decl->getIdentifier())
        ReportDeclUse(CurrentLoc(), enum_decl);
      else
        ReportDeclUse(CurrentLoc(), enum_constant_decl);
    }
    return true;
  }

  // When we call (or potentially call) a function, do an IWYU check
  // via ReportDeclUse() to make sure the definition of the function
  // is properly #included.
  bool HandleFunctionCall(FunctionDecl* callee, const Type* parent_type,
                          const Expr* calling_expr) {
    if (!Base::HandleFunctionCall(callee, parent_type, calling_expr))
      return false;
    if (!callee || CanIgnoreCurrentASTNode() || CanIgnoreDecl(callee))
      return true;

    if (!IsImplicitlyInstantiatedDfn(callee) && !IsTemplatizedType(parent_type))
      HandleFnReturnOnCallSite(callee);

    // We may have already been checked in a previous
    // VisitOverloadExpr() call.  Don't check again in that case.
    if (IsProcessedOverloadLoc(CurrentLoc()))
      return true;

    ReportDeclUse(CurrentLoc(), callee);

    return true;
  }

  // The C++ spec does not allow us to apply typeid to an incomplete type.
  // Therefore, we must report the type to be included.
  bool VisitCXXTypeidExpr(CXXTypeidExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;

    QualType type;
    if (expr->isTypeOperand()) {
      type = expr->getTypeOperandSourceInfo()->getType();
    } else {
      type = expr->getExprOperand()->getType();
    }

    ReportTypeUse(CurrentLoc(), type.getTypePtr(), DerefKind::RemoveRefs);
    return true;
  }

  bool TraverseCXXDefaultArgExpr(CXXDefaultArgExpr*) {
    // They should already be handled at the function declaration site,
    // not at the call site.
    return true;
  }

  //------------------------------------------------------------
  // Visitors of types derived from Type.

  bool VisitType(Type* type) {
    // In VisitFunctionDecl(), we say all children of function
    // declarations are forward-declarable.  This is true, *except*
    // for the exception (throw) types.  We clean that up here.
    // TODO(csilvers): figure out how to do these two steps in one place.
    const FunctionProtoType* fn_type = nullptr;
    if (!fn_type) {
      fn_type = current_ast_node()->template GetParentAs<FunctionProtoType>();
    }
    if (!fn_type) {
      if (const FunctionDecl* fn_decl =
              current_ast_node()->template GetParentAs<FunctionDecl>())
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

    return true;
  }

  bool VisitTemplateSpecializationType(TemplateSpecializationType* type) {
    if (CanIgnoreCurrentASTNode() || CanIgnoreType(type))
      return true;

    const NamedDecl* decl = TypeToDeclAsWritten(type);

    // If we are forward-declarable, so are our template arguments.
    if (CanForwardDeclareType(current_ast_node())) {
      ReportDeclForwardDeclareUse(CurrentLoc(), decl);
      current_ast_node()->set_in_forward_declare_context(true);
    } else {
      if (type->isTypeAlias())
        ReportWrittenTypeAlias(type);
      else
        ReportDeclUse(CurrentLoc(), decl);
    }

    return true;
  }

  bool VisitTypedefType(TypedefType* type) {
    if (CanIgnoreCurrentASTNode())
      return true;
    if (!CanForwardDeclareType(current_ast_node()))
      ReportWrittenTypeAlias(type);
    return true;
  }

  bool VisitDecltypeType(DecltypeType* type) {
    if (CanIgnoreCurrentASTNode())
      return true;
    if (!CanForwardDeclareType(current_ast_node()))
      ReportTypeUse(CurrentLoc(), type, DerefKind::None);
    return true;
  }

  //------------------------------------------------------------
  // Visitors defined by BaseAstVisitor.

  bool VisitNestedNameSpecifier(NestedNameSpecifier* nns) {
    // If this is a NamespaceAlias, then mark the alias as used,
    // requiring whichever file(s) declare the alias.
    if (nns->getKind() == NestedNameSpecifier::NamespaceAlias) {
      ReportDeclUse(CurrentLoc(), nns->getAsNamespaceAlias());
    }

    if (!Base::VisitNestedNameSpecifier(nns))
      return false;
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
  // I1_TemplateClass<std::vector<I1_Class>> i1_nested_templateclass(...)
  void DetermineForwardDeclareStatusForTemplateArg(ASTNode* ast_node) {
    CHECK_(ast_node->IsA<TemplateArgument>() &&
           "Should only pass in a template arg to DFDSFTA");

    if (!IsDefaultTemplateTemplateArg(ast_node)) {
      ast_node->set_in_forward_declare_context(true);
    }
  }

  bool VisitTemplateArgument(const TemplateArgument& arg) {
    if (!Base::VisitTemplateArgument(arg))
      return false;
    // Template arguments are forward-declarable...usually.
    DetermineForwardDeclareStatusForTemplateArg(current_ast_node());
    return true;
  }

  bool VisitTemplateArgumentLoc(const TemplateArgumentLoc& argloc) {
    if (!Base::VisitTemplateArgumentLoc(argloc))
      return false;
    // Template arguments are forward-declarable...usually.
    DetermineForwardDeclareStatusForTemplateArg(current_ast_node());
    return true;
  }

  //------------------------------------------------------------
  // Helper routines for visiting and traversing.  These helpers
  // encode the logic of whether a particular type of object
  // can be forward-declared or not.

  // TODO(csilvers): get rid of in_forward_declare_context() and make
  // this the canonical place to figure out if we can forward-declare.
  bool CanForwardDeclareType(const ASTNode* ast_node) const {
    CHECK_(ast_node->IsA<Type>());
    if (const auto* type = ast_node->GetAs<Type>()) {
      if (const auto* enum_type = type->getAs<EnumType>()) {
        return CanBeOpaqueDeclared(enum_type);
      }
    }
    // If we're in a forward-declare context, well then, there you have it.
    if (ast_node->in_forward_declare_context())
      return true;

    // Another place we disregard what the language allows: if we're
    // a dependent type, in theory we can be forward-declared.  But
    // we require the full definition anyway, so all the template
    // callers don't have to provide it instead.  Thus we don't
    // run the following commented-out code (left here for reference):
    //if (ast_node->GetAs<TemplateSpecializationType>()->isDependentType())
    //  return false;

    // Read past elaborations like 'class' keyword or namespaces.
    ast_node = MostElaboratedAncestor(ast_node);

    // Now there are two options: either we are part of a type or we are part of
    // a declaration involving a type.
    const Type* parent_type = ast_node->GetParentAs<Type>();
    if (parent_type == nullptr) {
      // It's not a type; analyze different kinds of declarations.
      if (const auto *decl = ast_node->GetParentAs<ValueDecl>()) {
        // We can shortcircuit static data member and 'extern' variable
        // declarations immediately, they can always be forward-declared.
        if (const auto *var_decl = dyn_cast<VarDecl>(decl)) {
          if (!var_decl->isThisDeclarationADefinition() &&
              (var_decl->isStaticDataMember() ||
               var_decl->hasExternalStorage())) {
            return true;
          }
        }

        parent_type = GetTypeOf(decl);
      } else if (const auto* parent_decl = ast_node->GetParentAs<TagDecl>()) {
        // If we ourselves are a forward-decl -- that is, we're the type
        // component of a forward-declaration (which would be our parent
        // AST node) -- then we're forward-declarable by definition.
        if (IsForwardDecl(parent_decl))
          return true;
      }
    }

    // TODO(csilvers): should probably just be IsPointerOrReference
    return parent_type && IsPointerOrReferenceAsWritten(parent_type);
  }

 protected:
  const IwyuPreprocessorInfo& preprocessor_info() const {
    return visitor_state_->preprocessor_info;
  }

  set<const Type*> blocked_types_;

 private:
  template <typename T> friend class IwyuBaseAstVisitor;

  bool IsProcessedOverloadLoc(SourceLocation loc) const {
    return ContainsKey(visitor_state_->processed_overload_locs, loc);
  }

  void AddProcessedOverloadLoc(SourceLocation loc) {
    visitor_state_->processed_overload_locs.insert(loc);
  }

  // Report types needed for template instantiation in cases when template
  // specialization type isn't explicitly written in a source code
  // in a non-fwd-declarable context (otherwise, they should be reported from
  // VisitTemplateSpecializationType), e.g.:
  // template <class T> struct S { T t; };
  // void fn(const S<Class>& s) { // Forward declarations are sufficient here.
  //   (void)s.t; // Full 'Class' type is needed due to template instantiation.
  // }
  void ReportTplSpecComponentTypes(const TemplateSpecializationType*,
                                   const set<const Type*>& blocked_types) =
      delete;

  // Figures out if the type is provided by template specialization argument
  // e.g. when being 'using Type = TemplateArgument;' referred to as
  // 'Tpl<Providing>::Type'.
  set<const Type*> GetProvidedByTplArg(const Type*) = delete;

  // Returns 'true' if either argument or "destination" (currently, function
  // argument type if it is an argument construction) of 'CXXConstructExpr'
  // provides the constructed type. In that case, the type should not
  // be reported.
  bool SourceOrTargetTypeIsProvided(const ASTNode* construct_expr_node) const =
      delete;

  set<const Type*> GetProvidedTypes(const Type* type,
                                    SourceLocation loc) const {
    set<const Type*> retval;
    for (const Type* component : GetComponentsOfTypeWithoutSubstituted(type)) {
      // TODO(csilvers): if one of the intermediate typedefs
      // #includes the necessary definition of the 'final'
      // underlying type, do we want to return it here?
      if (const auto* typedef_type = dyn_cast<TypedefType>(component)) {
        InsertAllInto(
            GetProvidedTypesForAlias(typedef_type->desugar().getTypePtr(),
                                     GetLocation(typedef_type->getDecl())),
            &retval);
        continue;
      }
      if (const auto* tpl_spec_type =
              dyn_cast<TemplateSpecializationType>(component)) {
        const NamedDecl* decl = TypeToDeclAsWritten(tpl_spec_type);
        if (const auto* al_tpl_decl = dyn_cast<TypeAliasTemplateDecl>(decl)) {
          const TypeAliasDecl* al_decl = al_tpl_decl->getTemplatedDecl();
          InsertAllInto(
              GetProvidedTypesForAlias(tpl_spec_type->desugar().getTypePtr(),
                                       GetLocation(al_decl)),
              &retval);
          continue;
        }
      }
      const Type* canonical = GetCanonicalType(component);
      if (!CodeAuthorWantsJustAForwardDeclare(canonical, loc))
        retval.insert(canonical);
    }
    return retval;
  }

  void ReportTypeUseInternal(SourceLocation used_loc, const Type* type,
                             set<const Type*> blocked_types,
                             DerefKind deref_kind) {
    InsertAllInto(this->getDerived().GetProvidedByTplArg(type), &blocked_types);
    // It is important not to lose info about type aliases while desugaring
    // and dereferencing here, because they are handled further.
    type = Desugar(type);
    if (deref_kind == DerefKind::RemoveRefs ||
        deref_kind == DerefKind::RemoveRefsAndPtr) {
      if (const auto* ref_type = dyn_cast_or_null<ReferenceType>(type)) {
        // Keep deref_kind to remove possible additional references in type
        // alias chain.
        return ReportTypeUseInternal(
            used_loc, ref_type->getPointeeTypeAsWritten().getTypePtr(),
            blocked_types, deref_kind);
      }
    }
    if (deref_kind == DerefKind::RemoveRefsAndPtr) {
      if (const auto* ptr_type = dyn_cast_or_null<PointerType>(type)) {
        return ReportTypeUseInternal(used_loc,
                                     ptr_type->getPointeeType().getTypePtr(),
                                     blocked_types, DerefKind::None);
      }
    }

    if (CanIgnoreType(type))
      return;

    // Enum type uses can be ignored. Their size is known (either implicitly
    // 'int' or from a mandatory transitive inclusion of a non-fixed enum full
    // declaration, or explicitly using a C++ 11 enum base). Only if an enum
    // type or its enumerators are explicitly mentioned will they be reported
    // by IWYU from VisitTagType or VisitDeclRefExpr correspondingly.
    if (type->getAs<EnumType>())
      return;

    // Types in fwd-decl-context should be ignored here and reported from more
    // specialized places, i.e. when they are explicitly written. But in fact,
    // this check is redundant because TypeToDeclAsWritten returns nullptr for
    // pointers and references.
    if (IsPointerOrReferenceAsWritten(type))
      return;

    // For typedefs, the user of the type is sometimes the one
    // responsible for the underlying type.  We check if that is the
    // case here, since we might be using a typedef type from
    // anywhere.  ('autocast' is similar, but is handled in
    // VisitCastExpr; 'fn-return-type' is also similar and is
    // handled in HandleFunctionCall; default type template arguments are
    // handled in InstantiatedTemplateVisitor::CanIgnoreType.)
    if (const auto* typedef_type = type->getAs<TypedefType>()) {
      // One exception: if this TypedefType is being used in another
      // typedef (that is, 'typedef MyTypedef OtherTypdef'), then the
      // user -- the other typedef -- is never responsible for the
      // underlying type.  Instead, users of that typedef are.
      const ASTNode* ast_node = MostElaboratedAncestor(current_ast_node());
      if (!ast_node->ParentIsA<TypedefNameDecl>()) {
        const TypedefNameDecl* typedef_decl = typedef_type->getDecl();
        const Type* type = typedef_decl->getUnderlyingType().getTypePtr();
        const set<const Type*>& provided_with_typedef =
            GetProvidedTypesForAlias(type, GetLocation(typedef_decl));
        InsertAllInto(provided_with_typedef, &blocked_types);
        VERRS(6) << "User, not author, of typedef "
                 << typedef_decl->getQualifiedNameAsString()
                 << " owns the underlying type:\n";
        // If any of the used types are themselves typedefs, this will
        // result in a recursive expansion.
        ReportTypeUseInternal(used_loc, type, blocked_types, deref_kind);
      }
      return;
    }
    if (const auto* template_spec_type =
            type->getAs<TemplateSpecializationType>()) {
      if (template_spec_type->isTypeAlias()) {
        const NamedDecl* decl = TypeToDeclAsWritten(template_spec_type);
        if (const auto* al_tpl_decl = dyn_cast<TypeAliasTemplateDecl>(decl)) {
          const TypeAliasDecl* al_decl = al_tpl_decl->getTemplatedDecl();
          const Type* type = template_spec_type->getAliasedType().getTypePtr();
          InsertAllInto(GetProvidedTypesForAlias(type, GetLocation(al_decl)),
                        &blocked_types);
          ReportTypeUseInternal(used_loc, type, blocked_types, deref_kind);
        }
        return;
      }
    }

    // Map private types like __normal_iterator to their public counterpart.
    type = MapPrivateTypeToPublicType(type);
    // For the below, we want to be careful to call *our*
    // ReportDeclUse(), not any of the ones in subclasses.
    if (const auto* template_spec_type =
            dyn_cast<TemplateSpecializationType>(Desugar(type))) {
      this->getDerived().ReportTplSpecComponentTypes(template_spec_type,
                                                     blocked_types);
    }
    // Don't place 'blocked_types' check before 'ReportTplSpecComponentTypes'
    // because template may be provided (i. e. blocked) but its arguments may be
    // not.
    if (blocked_types.count(GetCanonicalType(type)))
      return;
    if (const NamedDecl* decl = TypeToDeclAsWritten(type)) {
      decl = GetDefinitionAsWritten(decl);
      VERRS(6) << "(For type " << PrintableType(type) << "):\n";
      IwyuBaseAstVisitor<Derived>::ReportDeclUse(used_loc, decl);
    }
  }

  void ReportWrittenTypeAlias(const Type* type) {
    // When the alias is from template (like 'Type' in
    // 'Tpl<Providing>::Type'), the type may be provided with template
    // argument and should not be reported. 'ReportTypeUse' performs that
    // analysis when passing 'ElaboratedType' into it.
    if (const auto* elaborated =
            current_ast_node()->template GetParentAs<ElaboratedType>()) {
      ReportTypeUse(CurrentLoc(), elaborated, DerefKind::None);
    } else {
      ReportTypeUse(CurrentLoc(), type, DerefKind::None);
    }
  }

  void ReportWithAdditionalBlockedTypes(
      const Type* type, const set<const Type*>& additional_blocked_types) {
    set<const Type*> new_blocked_types = additional_blocked_types;
    new_blocked_types.insert(blocked_types_.begin(), blocked_types_.end());
    ValueSaver<set<const Type*>> s(&blocked_types_, new_blocked_types);
    ReportTypeUse(CurrentLoc(), type, DerefKind::None);
  }

  void HandleFnReturnOnCallSite(const FunctionDecl* callee) {
    // Usually the function-author is responsible for providing the
    // full type information for the return type of the function, but
    // in cases where it's not, we have to take responsibility.
    // TODO(csilvers): check the fn argument types as well.
    const Type* return_type = callee->getReturnType().getTypePtr();
    if (IsPointerOrReferenceAsWritten(return_type))
      return;

    ReportWithAdditionalBlockedTypes(return_type,
                                     GetProvidedTypesForFnReturn(callee));
  }

  void HandleAutocastOnCallSite(CXXConstructExpr* expr) {
    // 'Autocast' -- calling a one-arg, non-explicit constructor
    // -- is a special case when it's done for a function call.
    // iwyu requires the function-writer to provide the #include
    // for the casted-to type, just so we don't have to require it
    // here.  *However*, the function-author can override this
    // iwyu requirement, in which case we're responsible for the
    // casted-to type.  See IwyuBaseASTVisitor::CanBeProvidedTypeComponent.
    const Type* type = expr->getType().getTypePtr();
    ReportWithAdditionalBlockedTypes(
        type, GetProvidedTypesForAutocast(current_ast_node()));
  }

  void HandleNonAutocastConstruction(clang::CXXConstructExpr* expr) {
    // Avoid reporting of class members from implicit constructor member
    // initializers: it is redundant and may be attributed to incorrect location
    // on macro expansion.
    if (IsCXXConstructExprInInitializer(current_ast_node()))
      return;
    if (this->getDerived().SourceOrTargetTypeIsProvided(current_ast_node()))
      return;
    ReportTypeUse(CurrentLoc(), expr->getType().getTypePtr(), DerefKind::None);
  }

  // Do not add any variables here!  If you do, they will not be shared
  // between the normal iwyu ast visitor and the
  // template-instantiation visitor, which is almost always a mistake.
  // Instead, add them to the VisitorState struct, above.
  VisitorState* const visitor_state_;
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

  InstantiatedTemplateVisitor(VisitorState* visitor_state)
      : Base(visitor_state) {
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

  // resugar_map is a map from an unsugared (canonicalized) template
  // type to the template type as written (or as close as we can find
  // to it).  If a type is not in resugar-map, it might be due to a
  // recursive template call and encode a template type we don't care
  // about ourselves.  If it's in the resugar_map but with a nullptr
  // value, it's a default template parameter, that the
  // template-caller may or may not be responsible for.
  void ScanInstantiatedFunction(
      const FunctionDecl* fn_decl, const Type* parent_type,
      const ASTNode* caller_ast_node,
      const map<const Type*, const Type*>& resugar_map,
      const set<const Type*>& blocked_types) {
    Clear();
    caller_ast_node_ = caller_ast_node;
    resugar_map_ = resugar_map;
    blocked_types_ = blocked_types;

    // Make sure that the caller didn't already put the decl on the ast-stack.
    CHECK_(caller_ast_node->GetAs<Decl>() != fn_decl && "AST node already set");
    // caller_ast_node requires a non-const ASTNode, but our node is
    // const.  This cast is safe because we don't do anything with this
    // node (instead, we immediately push a new node on top of it).
    set_current_ast_node(const_cast<ASTNode*>(caller_ast_node));

    TraverseExpandedTemplateFunctionHelper(fn_decl, parent_type);
  }

  // This isn't a Stmt, but sometimes we need to fully instantiate
  // a template class to get at a field of it, for instance:
  // MyClass<T>::size_type s;
  void ScanInstantiatedType(ASTNode* caller_ast_node,
                            const map<const Type*, const Type*>& resugar_map,
                            const set<const Type*>& blocked_types) {
    Clear();
    caller_ast_node_ = caller_ast_node;
    resugar_map_ = resugar_map;
    blocked_types_ = blocked_types;

    // The caller node *is* the current node, unlike ScanInstantiatedFunction
    // which instead starts in the context of the parent expression and relies
    // on a TraverseDecl call to push the decl to the top of the AST stack.
    set_current_ast_node(caller_ast_node);

    const TemplateSpecializationType* type =
        caller_ast_node->GetAs<TemplateSpecializationType>();
    CHECK_(type != nullptr && "Not a template specialization");

    // As in TraverseExpandedTemplateFunctionHelper, we ignore all AST nodes
    // that will be reported when we traverse the uninstantiated type.
    if (const NamedDecl* type_decl_as_written =
            GetDefinitionAsWritten(TypeToDeclAsWritten(type))) {
      AstFlattenerVisitor nodeset_getter(compiler());
      nodes_to_ignore_ = nodeset_getter.GetNodesBelow(
          const_cast<NamedDecl*>(type_decl_as_written));
    }

    TraverseTemplateSpecializationType(
        const_cast<TemplateSpecializationType*>(type));
  }

  //------------------------------------------------------------
  // Implements virtual methods from Base.

  // When checking a template instantiation, we don't care where the
  // template definition is, so we never have any reason to ignore a
  // node.
  bool CanIgnoreCurrentASTNode() const override {
    // TODO(csilvers): call CanIgnoreType() if we're a type.
    return nodes_to_ignore_.Contains(*current_ast_node());
  }

  // For template instantiations, we want to print the symbol even if
  // it's not from the main compilation unit.
  bool ShouldPrintSymbolFromCurrentFile() const override {
    return GlobalFlags().verbose >= 5;
  }

  string GetSymbolAnnotation() const override {
    return " in tpl";
  }

  bool CanIgnoreType(const Type* type, IgnoreKind ignore_kind =
                                           IgnoreKind::ForUse) const override {
    if (!type)
      return true;

    if (!IsTypeInteresting(type))
      return true;

    switch (ignore_kind) {
      case IgnoreKind::ForUse:
        if (!IsKnownTemplateParam(type))
          return true;
        break;
      case IgnoreKind::ForExpansion:
        if (!InvolvesKnownTemplateParam(type))
          return true;
        break;
    }

    // If we're a default template argument, we should ignore the type
    // if the template author intend-to-provide it, but otherwise we
    // should not ignore it -- the caller is responsible for the type.
    // This captures cases like hash_set<Foo>, where the caller is
    // responsible for defining hash<Foo>.
    // IsProvidedByTemplate handles the case we
    // have a templated class that #includes "foo.h" and has a
    // scoped_ptr<Foo>: we say the templated class provides Foo, even
    // though it's scoped_ptr.h that's actually trying to call
    // Foo::Foo and ::~Foo.
    // TODO(csilvers): this isn't ideal: ideally we'd want
    // 'TheInstantiatedTemplateForWhichTypeWasADefaultTemplateArgumentIntendsToProvide',
    // but clang doesn't store that information.
    return IsDefaultTemplateParameter(type) && IsProvidedByTemplate(type);
  }

  bool IsTypeInteresting(const Type* type) const {
    // We only care about types that would have been dependent in the
    // uninstantiated template: that is, SubstTemplateTypeParmType types
    // or types derived from them. We use nodes_to_ignore_ to select down
    // to those.
    return !nodes_to_ignore_.Contains(type);
  }

  bool IsKnownTemplateParam(const Type* type) const {
    // Among all subst-type params, we only want those in the resugar-map. If
    // we're not in the resugar-map at all, we're not a type corresponding to
    // the template being instantiated, so we can be ignored.
    type = GetCanonicalType(type);
    return ContainsKey(resugar_map_, type);
  }

  bool InvolvesKnownTemplateParam(const Type* type) const {
    return InvolvesTypeForWhich(
        type, [&](const Type* type) { return IsKnownTemplateParam(type); });
  }

  // We ignore function calls in nodes_to_ignore_, which were already
  // handled by the template-as-written, and function names that we
  // are not responsible for because the template code is (for
  // instance, we're not responsible for a vector's call to
  // allocator::allocator(), because <vector> provides it for us).
  bool CanIgnoreDecl(const Decl* decl) const override {
    if (!decl)
      return true;
    return nodes_to_ignore_.Contains(decl);
  }

  // We always attribute type uses to the template instantiator.  For
  // decls, we do unless it looks like the template "intends to
  // provide" the decl, by #including the file that defines the decl
  // (if templates call other templates, we have to find the right
  // template).
  void ReportDeclUse(SourceLocation used_loc, const NamedDecl* decl,
                     const char* comment = nullptr,
                     UseFlags extra_use_flags = 0) override {
    const SourceLocation actual_used_loc = GetLocOfTemplateThatProvides(decl);
    // Report use only if template doesn't itself provide the declaration.
    if (!actual_used_loc.isValid() ||
        GetFileEntry(actual_used_loc) == GetFileEntry(caller_loc())) {
      // Let all the currently active types and decls know about this
      // report, so they can update their cache entries.
      for (CacheStoringScope* storer : cache_storers_)
        storer->NoteReportedDecl(decl);
      Base::ReportDeclUse(caller_loc(), decl, comment, extra_use_flags);
    }
  }

  void ReportDeclForwardDeclareUse(SourceLocation, const NamedDecl*,
                                   const char* /*comment*/ = nullptr) override {
    // Forward declarations make sense only when a type is explicitly written.
    // But 'InstantiatedTemplateVisitor' is to traverse implicit template
    // specializations, actually. (Template definitions and explicit
    // specializations are handled by 'IwyuAstConsumer'). Because it traverses
    // implicit stuff, it should not suggest forward declarations.
  }

  void ReportTypeUse(SourceLocation used_loc, const Type* type,
                     DerefKind deref_kind) override {
    if (!type)
      return;
    // Don't bother about type aliases here: only template argument types
    // can be reported currently from here, but their provided types should be
    // in 'blocked_types_'.
    if (deref_kind == DerefKind::RemoveRefs ||
        deref_kind == DerefKind::RemoveRefsAndPtr) {
      if (type->isReferenceType())
        type = type->getPointeeType().getTypePtr();
    }
    if (deref_kind == DerefKind::RemoveRefsAndPtr) {
      if (type->isPointerType()) {
        type = type->getPointeeType().getTypePtr();
      }
    }
    // clang desugars template types, so Foo<MyTypedef>() gets turned
    // into Foo<UnderlyingType>().  Try to convert back.
    type = ResugarType(type);
    if (CanIgnoreType(type))
      return;

    if (!blocked_types_.count(GetCanonicalType(type))) {
      for (CacheStoringScope* storer : cache_storers_)
        storer->NoteReportedType(type);
    }
    Base::ReportTypeUse(caller_loc(), type, DerefKind::None);
  }

  //------------------------------------------------------------
  // Overridden traverse-style methods from Base.

  // The 'convenience' HandleFunctionCall is perfect for us!
  bool HandleFunctionCall(FunctionDecl* callee, const Type* parent_type,
                          const Expr* calling_expr) {
    if (const Type* resugared_type = ResugarType(parent_type))
      parent_type = resugared_type;
    if (!Base::HandleFunctionCall(callee, parent_type, calling_expr))
      return false;
    if (!callee || CanIgnoreCurrentASTNode() || CanIgnoreDecl(callee))
      return true;
    return TraverseExpandedTemplateFunctionHelper(callee, parent_type);
  }

  bool TraverseUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr* expr) {
    if (!Base::TraverseUnaryExprOrTypeTraitExpr(expr))
      return false;
    if (CanIgnoreCurrentASTNode())
      return true;
    const Type* arg_type = expr->getTypeOfArgument().getTypePtr();
    // Calling sizeof on a reference-to-X is the same as calling it on X.
    if (const auto* reftype = arg_type->getAs<ReferenceType>()) {
      arg_type = reftype->getPointeeTypeAsWritten().getTypePtr();
    }
    if (const auto* type = arg_type->getAs<TemplateSpecializationType>()) {
      // Even though sizeof(MyClass<T>) only requires knowing how much
      // storage MyClass<T> takes, the language seems to require that
      // MyClass<T> be fully instantiated, even typedefs.  (Try
      // compiling 'template<class T> struct C { typedef typename T::a t; };
      // class S; int main() { return sizeof(C<S>); }'.)
      return TraverseDataAndTypeMembersOfClassHelper(type);
    }
    return true;
  }

  bool TraverseTemplateSpecializationTypeHelper(
      const TemplateSpecializationType* type) {
    if (CanIgnoreCurrentASTNode())
      return true;

    if (type->isTypeAlias())
      return TraverseType(type->getAliasedType());

    // Skip the template traversal if this occurrence of the template name is
    // just a class qualifier for an out of line method, as opposed to an object
    // instantiation, where the templated code would need to be inspected.
    //
    // Class<T,U>::method() {
    // |-Type---^
    // |-NNS------^
    // |-CXXMethodDecl--^
    ASTNode* ast_node = current_ast_node();
    if (const auto* nns = ast_node->GetParentAs<NestedNameSpecifier>()) {
      if (nns->getAsType() == type) {
        if (const auto* method = ast_node->GetAncestorAs<CXXMethodDecl>(2)) {
          CHECK_(nns == method->getQualifier());
          return true;
        }
      }
    }

    if (CanForwardDeclareType(ast_node))
      ast_node->set_in_forward_declare_context(true);
    return TraverseDataAndTypeMembersOfClassHelper(type);
  }

  bool TraverseTemplateSpecializationType(TemplateSpecializationType* type) {
    if (!Base::TraverseTemplateSpecializationType(type))
      return false;
    return TraverseTemplateSpecializationTypeHelper(type);
  }

  bool TraverseTemplateSpecializationTypeLoc(
      TemplateSpecializationTypeLoc typeloc) {
    if (!Base::TraverseTemplateSpecializationTypeLoc(typeloc))
      return false;
    return TraverseTemplateSpecializationTypeHelper(typeloc.getTypePtr());
  }

  // This helper is called on every use of a template argument type in an
  // instantiated template.  Its goal is to determine whether that use should
  // constitute a full-use by the template caller, and perform other necessary
  // bookkeeping.
  void AnalyzeTemplateTypeParmUse(const Type* type) {
    // Figure out how this type was actually written.  clang always
    // canonicalizes SubstTemplateTypeParmType, losing typedef info, etc.
    const Type* actual_type = ResugarType(type);
    CHECK_(actual_type &&
           "Type passed to AnalyzeTemplateTypeParmUse should be resugar-able");

    VERRS(6) << "AnalyzeTemplateTypeParmUse: type = " << PrintableType(type)
             << ", actual_type = " << PrintableType(actual_type) << '\n';

    if (CanForwardDeclareType(MostElaboratedAncestor(current_ast_node()))) {
      // Non-full uses will already have been reported when they were used as
      // template arguments, so nothing to do here.
      return;
    }

    // At this point we know we are looking at a full-use of type.  However,
    // there are still two cases.  If this is a template param that was written
    // by the user, then we report a use of it.  However, it might also be a
    // parameter of some implementation detail template which just happens top
    // involve the user's template parameter somehow.  In this case, we need to
    // recursively explore the instantiation of *that* template to see if the
    // user's type is full-used therein.

    if (IsKnownTemplateParam(type)) {
      // We attribute all uses in an instantiated template to the
      // template's caller.
      ReportTypeUse(caller_loc(), type, DerefKind::None);

      // Also report all previous explicit instantiations (declarations and
      // definitions) as uses of the caller's location.
      ReportExplicitInstantiations(type);
    } else {
      const Decl* decl = TypeToDeclAsWritten(type);
      if (const auto* cts_decl =
              dyn_cast<ClassTemplateSpecializationDecl>(decl)) {
        if (ContainsKey(traversed_decls_, decl))
          return;  // avoid recursion & repetition
        traversed_decls_.insert(decl);

        VERRS(6) << "Recursively traversing " << PrintableDecl(cts_decl)
                 << " which was full-used and does not involve a known"
                 << " template param\n";
        TraverseCXXRecordDecl(
            const_cast<ClassTemplateSpecializationDecl*>(cts_decl));
      }
    }
  }

  // Our task is made easier since (at least in theory), every time we
  // instantiate a template type, the instantiation has type
  // SubstTemplateTypeParmTypeLoc in the AST tree.
  bool VisitSubstTemplateTypeParmType(SubstTemplateTypeParmType* type) {
    if (CanIgnoreCurrentASTNode() ||
        CanIgnoreType(type, IgnoreKind::ForExpansion))
      return true;

    AnalyzeTemplateTypeParmUse(type);

    return Base::VisitSubstTemplateTypeParmType(type);
  }

  bool VisitTemplateSpecializationType(TemplateSpecializationType* type) {
    if (CanIgnoreCurrentASTNode())
      return true;

    CHECK_(current_ast_node()->GetAs<TemplateSpecializationType>() == type)
        << "The current node must be equal to the template spec. type";

    // Report previous explicit instantiations here, only if the type is needed
    // fully.
    if (!CanForwardDeclareType(current_ast_node()))
      ReportExplicitInstantiations(type);

    return Base::VisitTemplateSpecializationType(type);
  }

  void ReportExplicitInstantiations(const Type* type) {
    const auto* decl = dyn_cast_or_null<ClassTemplateSpecializationDecl>(
        TypeToDeclAsWritten(type));
    if (decl == nullptr)
      return;

    if (IsProvidedByTemplate(decl))
      return;

    // The caller loc is sometimes invalid due to ambiguity under macros. See
    // ASTNode::GetLocation() for more details.
    if (caller_loc().isInvalid()) {
      VERRS(6) << "Ignoring explicit instantiation of '" << PrintableDecl(decl)
               << "'; caller loc is invalid\n";
      return;
    }

    // Go through all previous redecls and filter out those that are not
    // explicit template instantiations or already provided by the template.
    std::vector<const CXXRecordDecl*> explicit_inst_decls;
    for (const NamedDecl* redecl : decl->redecls()) {
      if (!IsExplicitInstantiation(redecl)) {
        continue;
      }

      // Must have a valid location to find preceding decls. We have not seen
      // invalid locations in practice here, so assert.
      SourceLocation redecl_loc = redecl->getLocation();
      CHECK_(redecl_loc.isValid());

      if (IsBeforeInTranslationUnit(redecl_loc, caller_loc())) {
        // Earlier checks imply that this is a CXXRecordDecl.
        explicit_inst_decls.push_back(cast<CXXRecordDecl>(redecl));
      }
    }

    if (explicit_inst_decls.empty())
      return;

    // If there is a redecl in the same file, prefer that.
    for (const CXXRecordDecl* redecl : explicit_inst_decls) {
      if (GetFileEntry(redecl->getLocation()) == GetFileEntry(caller_loc())) {
        VERRS(6) << "Found explicit instantiation declaration or definition in "
                    "same file\n";
        Base::ReportDeclUse(caller_loc(), redecl,
                            "(for explicit instantiation)",
                            UF_ExplicitInstantiation);
        return;
      }
    }

    // Otherwise, if there is any redecl that is an explicit instantiation
    // _declaration_, prefer that, because a declaration should be cheaper to
    // process than a definition. The language guarantees that there is a
    // definition available somewhere in the program, or the linker will
    // complain.
    for (const CXXRecordDecl* redecl : explicit_inst_decls) {
      if (redecl->getTemplateSpecializationKind() ==
          clang::TSK_ExplicitInstantiationDeclaration) {
        VERRS(6) << "Found explicit instantiation declaration\n";
        Base::ReportDeclUse(caller_loc(), redecl,
                            "(for explicit instantiation)",
                            UF_ExplicitInstantiation);
        return;
      }
    }

    // If nothing more specific was found, arbitrarily pick the first one.
    if (explicit_inst_decls.size() > 1) {
      VERRS(6) << "Found " << explicit_inst_decls.size() << " "
               << "explicit instantiation decls; reporting the first one\n";
    }
    Base::ReportDeclUse(caller_loc(), explicit_inst_decls[0],
                        "(for explicit instantiation)",
                        UF_ExplicitInstantiation);
  }

  // If constructing an object, check the type we're constructing.
  // Normally we'd see that type later, when traversing the return
  // type of the constructor-decl, but if we wait for that, we'll lose
  // any SubstTemplateTypeParmType's we have (we lose all
  // SubstTemplateTypeParmType's going from Expr to Decl).
  // TODO(csilvers): This should maybe move to HandleFunctionCall.
  bool VisitCXXConstructExpr(CXXConstructExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;
    const Type* class_type = GetTypeOf(expr);
    if (CanIgnoreType(class_type))
      return true;

    // If the ctor type is a SubstTemplateTypeParmType, get the type-as-written.
    const Type* actual_type = ResugarType(class_type);
    CHECK_(actual_type && "If !CanIgnoreType(), we should be resugar-able");
    ReportTypeUse(caller_loc(), actual_type, DerefKind::None);
    return Base::VisitCXXConstructExpr(expr);
  }

  // --- Handler declared in IwyuBaseASTVisitor.

  void ReportTplSpecComponentTypes(const TemplateSpecializationType* type,
                                   const set<const Type*>& /*blocked_types*/) {
    // TODO(bolshakov): should 'blocked_types' argument be considered here?
    TraverseDataAndTypeMembersOfClassHelper(type);
  }

  set<const Type*> GetProvidedByTplArg(const Type*) {
    // Already inside template instantiation analysis. Types provided
    // by template arguments should be in 'blocked_types_' set.
    return set<const Type*>{};
  }

  bool SourceOrTargetTypeIsProvided(const ASTNode* construct_expr_node) const {
    // During instantiated template analysis, provided types are extracted
    // from template arguments and are in 'blocked_types_' set.
    return false;
  }

 private:
  // Clears the state of the visitor.
  void Clear() {
    caller_ast_node_ = nullptr;
    resugar_map_.clear();
    traversed_decls_.clear();
    nodes_to_ignore_.clear();
    cache_storers_.clear();
  }

  // If we see the instantiated template using a type or decl (such as
  // std::allocator), we want to know if the author of the template is
  // providing the type or decl, so the code using the instantiated
  // template doesn't have to.  For instance:
  //    vector<int, /*allocator<int>*/> v;   // in foo.cc
  // Does <vector> provide the definition of allocator<int>?  If not,
  // foo.cc will have to #include <allocator>.
  //   We say the template-as-written does provide the decl if it, or
  // any other header seen since we started instantiating the
  // template, sees it.  The latter requirement is to deal with a
  // situation like this: we have a templated class that #includes
  // "foo.h" and has a scoped_ptr<Foo>; we say the templated class
  // provides Foo, even though it's scoped_ptr.h that's actually
  // trying to call Foo::Foo and Foo::~Foo.
  SourceLocation GetLocOfTemplateThatProvides(const NamedDecl* decl) const {
    if (!decl)
      return SourceLocation();   // an invalid source-loc
    for (const ASTNode* ast_node = current_ast_node();
         ast_node != caller_ast_node_; ast_node = ast_node->parent()) {
      if (preprocessor_info().PublicHeaderIntendsToProvide(
              GetFileEntry(ast_node->GetLocation()),
              GetFileEntry(decl->getLocation())))
        return ast_node->GetLocation();
    }
    return SourceLocation();   // an invalid source-loc
  }

  bool IsProvidedByTemplate(const NamedDecl* decl) const {
    return GetLocOfTemplateThatProvides(decl).isValid();
  }
  bool IsProvidedByTemplate(const Type* type) const {
    type = Desugar(type);
    type = RemovePointersAndReferences(type);  // get down to the decl
    if (const NamedDecl* decl = TypeToDeclAsWritten(type)) {
      decl = GetDefinitionAsWritten(decl);
      return GetLocOfTemplateThatProvides(decl).isValid();
    }
    return true;   // we always provide non-decl types like int, etc.
  }

  // For a SubstTemplateTypeParmType, says whether it corresponds to a
  // default template parameter (one not explicitly specified when the
  // class was instantiated) or not.  We store this in resugar_map by
  // having the value be nullptr.
  bool IsDefaultTemplateParameter(const Type* type) const {
    type = GetCanonicalType(type);
    return ContainsKeyValue(resugar_map_, type, static_cast<Type*>(nullptr));
  }

  // clang desugars template types, so Foo<MyTypedef>() gets turned
  // into Foo<UnderlyingType>().  We can 'resugar' using resugar_map_.
  // If we're not in the resugar-map, then we weren't canonicalized,
  // so we can just use the input type unchanged.
  const Type* ResugarType(const Type* type) const {
    type = GetCanonicalType(type);
    // If we're the resugar-map but with a value of nullptr, it means
    // we're a default template arg, which means we don't have anything
    // to resugar to.  So just return the input type.
    if (ContainsKeyValue(resugar_map_, type, static_cast<const Type*>(nullptr)))
      return type;
    return GetOrDefault(resugar_map_, type, type);
  }

  bool TraverseExpandedTemplateFunctionHelper(const FunctionDecl* fn_decl,
                                              const Type* parent_type) {
    if (!fn_decl || ContainsKey(traversed_decls_, fn_decl))
      return true;   // avoid recursion and repetition
    traversed_decls_.insert(fn_decl);

    // If we have cached the reporting done for this decl before,
    // report again (but with the new caller_loc this time).
    // Otherwise, for all reporting done in the rest of this scope,
    // store in the cache for this function.
    if (ReplayUsesFromCache(*FunctionCallsFullUseCache(),
                            fn_decl, caller_loc()))
      return true;
    // Make sure all the types we report in the recursive TraverseDecl
    // calls, below, end up in the cache for fn_decl.
    CacheStoringScope css(&cache_storers_, FunctionCallsFullUseCache(),
                          fn_decl, resugar_map_);

    // We want to ignore all nodes that are the same in this
    // instantiated function as they are in the uninstantiated version
    // of the function.  The latter will be reported when we traverse
    // the uninstantiated function, so we don't need to re-traverse
    // them here.
    AstFlattenerVisitor nodeset_getter(compiler());
    // This gets to the decl for the (uninstantiated) template-as-written:
    const FunctionDecl* decl_as_written =
        fn_decl->getTemplateInstantiationPattern();
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

    // We need to iterate over the function.
    return TraverseDecl(const_cast<FunctionDecl*>(fn_decl));
  }

  // Does the actual recursing over data members and type members of
  // the instantiated class.  Unlike
  // TraverseClassTemplateSpecializationDecl() in the base class, it
  // does *not* traverse the methods.
  bool TraverseDataAndTypeMembersOfClassHelper(
      const TemplateSpecializationType* type) {
    if (!type)
      return true;

    // No point in doing traversal if we're forward-declared
    if (current_ast_node()->in_forward_declare_context())
      return true;

    // If we're a dependent type, we only try to be analyzed if we're
    // in the precomputed list -- in general, the only thing clang
    // tells us about dependent types is their name (which is all we
    // need for the precomputed list!).  This means iwyu will properly
    // analyze the use of SomeClass in code like 'map<T, SomeClass>',
    // but not in 'MyMap<T, SomeClass>', since we have precomputed
    // information about the STL map<>, but not the user type MyMap.
    // TODO(csilvers): do better here.
    if (type->isDependentType()) {
      // TODO(csilvers): This is currently always a noop; need to fix
      // GetTplInstDataForClassNoComponentTypes to do something
      // useful for dependent types.
      ReplayClassMemberUsesFromPrecomputedList(type);   // best-effort
      return true;
    }

    const NamedDecl* named_decl = TypeToDeclAsWritten(type);
    const ClassTemplateSpecializationDecl* class_decl = DynCastFrom(named_decl);

    // Bail out if we are not a proper class
    if (class_decl == nullptr) {
      // If the template specialization decl is not sugar for a class, we
      // expect it to be another kind of template decl, like a built-in.
      // Also in some rare cases named_decl can be a record decl (e.g. when
      // using the built-in __type_pack_element).
      CHECK_(llvm::isa<TemplateDecl>(named_decl) ||
             llvm::isa<RecordDecl>(named_decl))
          << "TemplateSpecializationType has no decl of type TemplateDecl or "
             "RecordDecl?";
      return true;
    }

    if (ContainsKey(traversed_decls_, class_decl))
      return true;   // avoid recursion & repetition
    traversed_decls_.insert(class_decl);

    // If we have cached the reporting done for this decl before,
    // report again (but with the new caller_loc this time).
    // Otherwise, for all reporting done in the rest of this scope,
    // store in the cache for this function.
    if (ReplayUsesFromCache(*ClassMembersFullUseCache(),
                            class_decl, caller_loc()))
      return true;
    if (ReplayClassMemberUsesFromPrecomputedList(type))
      return true;

    // Sometimes, an implicit specialization occurs to be not instantiated.
    // TODO(bolshakov): don't report them at all as full uses or figure out
    // how to scan them.
    if (!class_decl->hasDefinition())
      return true;

    // Make sure all the types we report in the recursive TraverseDecl
    // calls, below, end up in the cache for class_decl.
    CacheStoringScope css(&cache_storers_, ClassMembersFullUseCache(),
                          class_decl, resugar_map_);

    for (DeclContext::decl_iterator it = class_decl->decls_begin();
         it != class_decl->decls_end(); ++it) {
      if (isa<CXXMethodDecl>(*it) || isa<FunctionTemplateDecl>(*it))
        continue;
      if (!TraverseDecl(*it))
        return false;
    }

    for (const CXXBaseSpecifier& base : class_decl->bases()) {
      if (!TraverseType(base.getType()))
        return false;
    }

    // Most methods on template classes are instantiated when they're
    // called, and we don't need to deal with them here.  But virtual
    // methods are instantiated when the class's key method is
    // instantiated, and since template classes rarely have a key
    // method, it means they're instantiated whenever the class is
    // instantiated.  So we need to instantiate virtual methods here.
    for (DeclContext::decl_iterator it = class_decl->decls_begin();
         it != class_decl->decls_end(); ++it) {
      if (const CXXMethodDecl* method_decl = DynCastFrom(*it)) {
        if (method_decl->isVirtual()) {
          if (!TraverseExpandedTemplateFunctionHelper(method_decl, type))
            return false;
        }
      }
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
    if (!cache.Contains(key, resugar_map_))
      return false;
    VERRS(6) << "(Replaying full-use information from the cache for "
             << key->getQualifiedNameAsString() << ")\n";
    ReportTypesUse(use_loc, cache.GetFullUseTypes(key, resugar_map_));
    ReportDeclsUse(use_loc, cache.GetFullUseDecls(key, resugar_map_));
    return true;
  }

  // We precompute (hard-code) results of calling
  // TraverseDataAndTypeMembersOfClassHelper for some types (mostly
  // STL types).  This way we don't even need to traverse them once.
  // Returns true iff we did appropriate reporting for this type.
  bool ReplayClassMemberUsesFromPrecomputedList(
      const TemplateSpecializationType* tpl_type) {
    if (current_ast_node() && current_ast_node()->in_forward_declare_context())
      return true;   // never depend on any types if a fwd-decl

    const NamedDecl* tpl_decl = TypeToDeclAsWritten(tpl_type);

    // This says how the template-args are used by this hard-coded type
    // (a set<>, or map<>, or ...), to avoid having to recurse into them.
    const map<const Type*, const Type*>& resugar_map_for_precomputed_type =
        FullUseCache::GetPrecomputedResugarMap(tpl_type,
                                               compiler()->getLangOpts());
    // But we need to reconcile that with the types-of-interest, as
    // stored in resugar_map_.  To do this, we take only those entries
    // from resugar_map_for_precomputed_type that are also present in
    // resugar_map_.  We consider type components, so if
    // resugar_map_for_precomputed_type has less_than<Foo> or hash<Foo>,
    // we'll add those in even if resugar_map_ only includes 'Foo'.
    map<const Type*, const Type*> resugar_map;
    for (const auto& item : resugar_map_for_precomputed_type) {
      // TODO(csilvers): for default template args, item.first is sometimes
      // a RecordType even when it's a template specialization.  Figure out
      // how to get the proper type components in that situation.
      const set<const Type*> type_components = GetComponentsOfType(item.first);
      if (ContainsAnyKey(resugar_map_, type_components)) {
        resugar_map.insert(item);
      }
    }
    if (resugar_map.empty())
      return false;

    VERRS(6) << "(Using pre-computed list of full-use information for "
             << tpl_decl->getQualifiedNameAsString() << ")\n";
    // For entries with a non-nullptr value, we report the value, which
    // is the unsugared type, as being fully used.  Entries with a
    // nullptr value are default template args, and we only report them
    // if the template class doesn't intend-to-provide them.
    for (const auto& item : resugar_map) {
      const Type* resugared_type = nullptr;
      if (item.second) {
        resugared_type = item.second;
      } else {
        const NamedDecl* resugared_decl = TypeToDeclAsWritten(item.first);
        if (!preprocessor_info().PublicHeaderIntendsToProvide(
                GetFileEntry(tpl_decl), GetFileEntry(resugared_decl)))
          resugared_type = item.first;
      }
      if (resugared_type && !resugared_type->isPointerType()) {
        ReportTypeUse(caller_loc(), resugared_type, DerefKind::None);
        // For a templated type, check the template args as well.
        if (const TemplateSpecializationType* spec_type =
                DynCastFrom(resugared_type)) {
          TraverseDataAndTypeMembersOfClassHelper(spec_type);
        }
      }
    }
    return true;
  }

  //------------------------------------------------------------
  // Member accessors.

  SourceLocation caller_loc() const {
    return caller_ast_node_->GetLocation();
  }

  //------------------------------------------------------------
  // Member variables.

  // The AST-chain when this template was instantiated.
  const ASTNode* caller_ast_node_;

  // resugar_map is a map from an unsugared (canonicalized) template
  // type to the template type as written (or as close as we can find
  // to it).  If a type is not in resugar-map, it might be due to a
  // recursive template call and encode a template type we don't care
  // about ourselves.  If it's in the resugar_map but with a nullptr
  // value, it's a default template parameter, that the
  // template-caller may or may not be responsible for.
  map<const Type*, const Type*> resugar_map_;

  // Used to avoid recursion in the *Helper() methods.
  set<const Decl*> traversed_decls_;

  AstFlattenerVisitor::NodeSet nodes_to_ignore_;

  // The current set of nodes we're updating cache entries for.
  set<CacheStoringScope*> cache_storers_;
};  // class InstantiatedTemplateVisitor

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

  IwyuAstConsumer(VisitorState* visitor_state)
      : Base(visitor_state),
        instantiated_template_visitor_(visitor_state) {}

  //------------------------------------------------------------
  // Implements pure virtual methods from Base.

  // Returns true if we are not interested in symbols used in used_in
  // for whatever reason.  For instance, we can ignore nodes that are
  // neither in the file we're compiling nor in its associated .h file.
  bool CanIgnoreCurrentASTNode() const override {
    // If we're outside of foo.{h,cc} and the set of check_also files,
    // just ignore.
    if (CanIgnoreLocation(current_ast_node()->GetLocation()))
      return true;

    // TODO(csilvers): if we're a type, call CanIgnoreType().

    return false;
  }

  // We print symbols from files in the main compilation unit (foo.cc,
  // foo.h, foo-inl.h) if the debug level is 5 or 6, for non-system
  // files if the debug level is 7, and all files if the debug level
  // is 8 or more.
  bool ShouldPrintSymbolFromCurrentFile() const override {
    return ShouldPrintSymbolFromFile(CurrentFileEntry());
  }

  string GetSymbolAnnotation() const override {
    return "";
  }

  // We are interested in all types for iwyu checking.
  bool CanIgnoreType(const Type* type, IgnoreKind) const override {
    return type == nullptr;
  }

  bool CanIgnoreDecl(const Decl* decl) const override {
    return decl == nullptr;
  }

  //------------------------------------------------------------
  // Parser event handlers.  Clang will call them to notify this
  // ASTConsumer as it parses the source code.  See class ASTConsumer in
  // clang/AST/ASTConsumer.h
  // for all the handlers we can override.

  // Called once at the beginning of the compilation.
  void Initialize(ASTContext& context) override {}  // NOLINT

  // Called once at the end of the compilation.
  void HandleTranslationUnit(ASTContext& context) override {  // NOLINT
    // TODO(csilvers): automatically detect preprocessing is done, somehow.
    const_cast<IwyuPreprocessorInfo*>(&preprocessor_info())->
        HandlePreprocessingDone();

    TranslationUnitDecl* tu_decl = context.getTranslationUnitDecl();

    // Sema::TUScope is reset after parsing, but Sema::getCurScope still points
    // to the translation unit decl scope. TUScope is required for lookup in
    // some complex scenarios, so re-wire it before running IWYU AST passes.
    // Assert their expected state so we notice if these assumptions change.
    Sema& sema = compiler()->getSema();
    CHECK_(sema.TUScope == nullptr);
    CHECK_(sema.getCurScope() != nullptr);
    sema.TUScope = sema.getCurScope();

    // We run a separate pass to force parsing of late-parsed function
    // templates.
    ParseFunctionTemplates(sema, tu_decl);

    // Clang lazily constructs the implicit methods of a C++ class (the
    // default constructor and destructor, etc) -- it only bothers to
    // create a CXXMethodDecl if someone actually calls these classes.
    // But we need to be non-lazy: IWYU depends on analyzing what future
    // code *may* call in a class, not what current code *does*.  So we
    // force all the lazy evaluation to happen here.
    InstantiateImplicitMethods(sema, tu_decl);

    // Run IWYU analysis.
    TraverseDecl(tu_decl);

    // Check if any unrecoverable errors have occurred.
    // There is no point in continuing when the AST is in a bad state.
    if (compiler()->getDiagnostics().hasUnrecoverableErrorOccurred())
      exit(EXIT_FAILURE);

    const set<OptionalFileEntryRef>* const files_to_report_iwyu_violations_for =
        preprocessor_info().files_to_report_iwyu_violations_for();

    // Some analysis, such as UsingDecl resolution, is deferred until the
    // entire AST is visited because it's only at that point that we know if
    // the symbol was actually used or not.
    // We perform that analysis here before CalculateAndReportIwyuViolations.
    for (OptionalFileEntryRef file : *files_to_report_iwyu_violations_for) {
      CHECK_(preprocessor_info().FileInfoFor(file));
      preprocessor_info().FileInfoFor(file)->ResolvePendingAnalysis();
    }

    // We have to calculate the .h files before the .cc file, since
    // the .cc file inherits #includes from the .h files, and we
    // need to figure out what those #includes are going to be.
    size_t num_edits = 0;
    OptionalFileEntryRef const main_file = preprocessor_info().main_file();
    for (OptionalFileEntryRef file : *files_to_report_iwyu_violations_for) {
      if (file == main_file)
        continue;
      CHECK_(preprocessor_info().FileInfoFor(file));
      num_edits += preprocessor_info().FileInfoFor(file)
          ->CalculateAndReportIwyuViolations();
    }
    CHECK_(preprocessor_info().FileInfoFor(main_file));
    num_edits += preprocessor_info().FileInfoFor(main_file)
        ->CalculateAndReportIwyuViolations();

    int exit_code = EXIT_SUCCESS;
    if (GlobalFlags().exit_code_always) {
      // If we should always fail, use --error_always value.
      exit_code = GlobalFlags().exit_code_always;
    } else if (num_edits > 0) {
      // If there were IWYU violations, use --error value.
      exit_code = GlobalFlags().exit_code_error;
    }

    exit(exit_code);
  }

  void ParseFunctionTemplates(Sema& sema, TranslationUnitDecl* tu_decl) {
    set<FunctionDecl*> late_parsed_decls = GetLateParsedFunctionDecls(tu_decl);

    // If we have any late-parsed functions, make sure the
    // -fdelayed-template-parsing flag is on. Otherwise we don't know where
    // they came from.
    CHECK_((compiler()->getLangOpts().DelayedTemplateParsing ||
            late_parsed_decls.empty()) &&
           "Should not have late-parsed decls without "
           "-fdelayed-template-parsing.");

    for (const FunctionDecl* fd : late_parsed_decls) {
      CHECK_(fd->isLateTemplateParsed());

      if (CanIgnoreLocation(GetLocation(fd)))
        continue;

      // Force parsing and AST building of the yet-uninstantiated function
      // template body.
      LateParsedTemplate* lpt = sema.LateParsedTemplateMap[fd].get();
      sema.LateTemplateParser(sema.OpaqueParser, *lpt);
    }
  }

  void InstantiateImplicitMethods(Sema& sema, TranslationUnitDecl* tu_decl) {
    // Collect all implicit ctors/dtors that need to be instantiated.
    struct Visitor : public RecursiveASTVisitor<Visitor> {
      Visitor(Sema& sema) : sema(sema) {
      }

      bool VisitCXXRecordDecl(CXXRecordDecl* decl) {
        if (CanIgnoreLocation(GetLocation(decl)))
          return true;

        if (!decl->getDefinition() || decl->isDependentContext() ||
            decl->isBeingDefined()) {
          return true;
        }

        if (CXXConstructorDecl* ctor = sema.LookupDefaultConstructor(decl)) {
          may_need_definition.insert(ctor);
        } else if (CXXDestructorDecl* dtor = sema.LookupDestructor(decl)) {
          may_need_definition.insert(dtor);
        }

        return true;
      }

      Sema& sema;
      std::set<CXXMethodDecl*> may_need_definition;
    };

    // Run visitor to collect implicit methods.
    Visitor v(sema);
    v.TraverseDecl(tu_decl);

    // For each method collected, let Sema define them.
    for (CXXMethodDecl* method : v.may_need_definition) {
      if (!method->isDefaulted() || method->isDeleted() || method->hasBody())
        continue;

      SourceLocation loc = GetLocation(method->getParent());
      if (auto* ctor = dyn_cast<CXXConstructorDecl>(method)) {
        sema.DefineImplicitDefaultConstructor(loc, ctor);
      } else if (auto* dtor = dyn_cast<CXXDestructorDecl>(method)) {
        sema.DefineImplicitDestructor(loc, dtor);
      }
    }

    // Clang queues up method instantiations. Process them now.
    sema.PerformPendingInstantiations();
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

  // --- Visitors of types derived from Decl.

  bool VisitNamespaceAliasDecl(NamespaceAliasDecl* decl) {
    if (CanIgnoreCurrentASTNode())
      return true;
    ReportDeclUse(CurrentLoc(), decl->getNamespace());
    return Base::VisitNamespaceAliasDecl(decl);
  }

  bool VisitUsingDecl(UsingDecl* decl) {
    // The shadow decls hold the declarations for the var/fn/etc we're
    // using.  (There may be more than one if, say, we're using an
    // overloaded function.)  We don't want to add all of them at once
    // though, because that will drag in every overload even if we're
    // only using one.  Instead, we keep track of the using decl and
    // mark it as touched when something actually uses it.
    preprocessor_info().FileInfoFor(CurrentFileEntry())->AddUsingDecl(decl);

    if (CanIgnoreCurrentASTNode())
      return true;
    return Base::VisitUsingDecl(decl);
  }

  bool VisitTagDecl(TagDecl* decl) {
    if (CanIgnoreCurrentASTNode())
      return true;

    // Skip the injected class name.
    if (decl->isImplicit())
      return Base::VisitTagDecl(decl);

    if (IsForwardDecl(decl)) {
      // If we're a templated class, make sure we add the whole template.
      const NamedDecl* decl_to_fwd_declare = decl;
      if (const CXXRecordDecl* cxx_decl = DynCastFrom(decl))
        if (cxx_decl->getDescribedClassTemplate())
          decl_to_fwd_declare = cxx_decl->getDescribedClassTemplate();

      // We've found a forward-declaration.  We'll report we've found
      // it, but we also want to report if we know already that we
      // should keep this forward-declaration around (and not consider
      // it for deletion if it's never used).  There are a few
      // situations we can do this, described below.
      bool definitely_keep_fwd_decl = false;

      // (1) If the forward-decl has a linkage spec ('extern "C"')
      // then it can't be removed, since that information probably
      // isn't encoded anywhere else.
      // (Surprisingly classes can have linkage specs! -- they are
      // applied to all static methods of the class.  See
      // http://msdn.microsoft.com/en-us/library/ms882260.aspx.)
      if (current_ast_node()->ParentIsA<LinkageSpecDecl>()) {
        definitely_keep_fwd_decl = true;

      // (2) GCC-style __attributes__ work the same way: we can't assume
      // that attributes are consistent between declarations, so we can't
      // remove a decl with attributes unless they're inherited, i.e. propagated
      // from another redeclaration as opposed to explicitly written.
      } else if (decl->hasAttrs()) {
        for (const Attr* attr : decl->getAttrs()) {
          if (!attr->isInherited()) {
            definitely_keep_fwd_decl = true;
            break;
          }
        }

      // (3) If we're a nested class ("class A { class SubA; };"),
      // then we can't necessary be removed either, since we're part
      // of the public API of the enclosing class -- it's illegal to
      // have a nested class and not at least declare it in the
      // enclosing class.  If the nested class is actually defined in
      // the enclosing class, then we're fine; if not, we need to keep
      // the first forward-declaration.
      } else if (IsNestedTagAsWritten(current_ast_node())) {
        if (!decl->getDefinition() || decl->getDefinition()->isOutOfLine()) {
          // TODO(kimgr): Member class redeclarations are illegal, per C++
          // standard DR85, so this check for first redecl can be removed.
          // Nested classes should always be definitely kept. More details here:
          // http://comments.gmane.org/gmane.comp.compilers.clang.scm/74782
          // GCC and MSVC both still allow redeclarations of nested classes,
          // though, so it seems hygienic to remove all but one.
          if (const NamedDecl* first_decl = GetFirstRedecl(decl)) {
            // Check if we're the decl with the smallest line number.
            if (decl == first_decl)
              definitely_keep_fwd_decl = true;
          }
        }
      } else if (preprocessor_info().ForwardDeclareIsMarkedKeep(decl)) {
        definitely_keep_fwd_decl = true;
      } else if (preprocessor_info().ForwardDeclareIsExported(decl)) {
        definitely_keep_fwd_decl = true;
      }

      preprocessor_info().FileInfoFor(CurrentFileEntry())->AddForwardDeclare(
          decl_to_fwd_declare, definitely_keep_fwd_decl);
    }
    return Base::VisitTagDecl(decl);
  }

  // If you specialize a template, you need a declaration of the
  // template you're specializing.  That is, for code like this:
  //    template <class T> struct Foo;
  //    template<> struct Foo<int> { ... };
  // we don't want iwyu to recommend removing the 'forward declare' of Foo.
  //
  // Additionally, this type of decl is also used to represent explicit template
  // instantiations. Only instantiation definitions causing instantiation of
  // member functions are handled here. Other instantiated parts of the template
  // are handled in 'VisitTemplateSpecializationType', as usual.
  bool VisitClassTemplateSpecializationDecl(
      ClassTemplateSpecializationDecl* decl) {
    if (!IsExplicitInstantiation(decl)) {
      if (!CanIgnoreCurrentASTNode()) {
        ReportDeclForwardDeclareUse(CurrentLoc(),
                                    decl->getSpecializedTemplate());
      }
    } else if (IsExplicitInstantiationDefinitionAsWritten(decl)) {
      // Explicit instantiation definition causes instantiation of all
      // the template methods. Scan them here to assure that all the needed
      // template argument types are '#include'd.
      const TypeLoc type_loc = decl->getTypeAsWritten()->getTypeLoc();
      if (CanIgnoreLocation(GetLocation(&type_loc)))
        return true;
      // Clang attributes 'ClassTemplateSpecializationDecl' to the original
      // template location. Construct a new node corresponding to the template
      // spec type location (as written) so that reportings from
      // 'InstantiatedTemplateVisitor' are attributed to the correct location.
      const ASTNode type_loc_node(&type_loc);
      const TemplateInstantiationData data =
          GetTplInstData(type_loc.getTypePtr());
      // Clang instantiates methods in the first ("canonical") spec decl context
      // (which may correspond to instantiation declaration, not to definition).
      for (const CXXMethodDecl* member : decl->getCanonicalDecl()->methods()) {
        instantiated_template_visitor_.ScanInstantiatedFunction(
            member, type_loc.getTypePtr(), &type_loc_node, data.resugar_map,
            data.provided_types);
      }
    }

    return Base::VisitClassTemplateSpecializationDecl(decl);
  }

  // Track use of namespace in using directive decl
  // a.h:
  //   namespace a { ... };
  //
  // b.cpp:
  //   include "a.h"
  //   using namespace a;
  //   ...
  bool VisitUsingDirectiveDecl(UsingDirectiveDecl* decl) {
    if (CanIgnoreCurrentASTNode())
      return true;
    ReportDeclUse(CurrentLoc(), decl->getNominatedNamespaceAsWritten());
    return Base::VisitUsingDirectiveDecl(decl);
  }

  bool AllTypesProvided(const Type* typedef_underlying_type,
                        SourceLocation typedef_decl_loc) const {
    for (const Type* type :
         GetComponentsOfTypeWithoutSubstituted(typedef_underlying_type)) {
      if (CodeAuthorWantsJustAForwardDeclare(type, typedef_decl_loc))
        return false;
    }
    return true;
  }

  // If you say 'typedef Foo Bar', then clients can use Bar however
  // they want without having to worry about #including anything
  // except you.  That puts you on the hook for all the #includes that
  // Bar might need, for *anything* one might want to do to a Bar.
  // TODO(csilvers): we can probably relax this rule in .cc files.
  // TODO(csilvers): this should really move into IwyuBaseASTVisitor
  // (that way we'll correctly identify need for hash<> in hash_set).
  // This is called from Traverse*() because Visit*()
  // can't call HandleFunctionCall().
  bool HandleAliasedClassMethods(TypedefNameDecl* decl) {
    if (CanIgnoreCurrentASTNode())
      return true;

    const Type* underlying_type = decl->getUnderlyingType().getTypePtr();
    if (!AllTypesProvided(underlying_type, current_ast_node()->GetLocation()))
      return true;
    const Decl* underlying_decl = TypeToDeclAsWritten(underlying_type);

    // We simulate a user calling all the methods in a class.
    if (const CXXRecordDecl* record_decl = DynCastFrom(underlying_decl)) {
      for (DeclContext::decl_iterator it = record_decl->decls_begin();
           it != record_decl->decls_end(); ++it) {
        FunctionDecl* fn_decl = nullptr;
        if (CXXMethodDecl* method_decl = DynCastFrom(*it)) {
          fn_decl = method_decl;
        } else if (FunctionTemplateDecl* tpl_decl = DynCastFrom(*it)) {
          fn_decl = tpl_decl->getTemplatedDecl();   // templated method decl
        } else {
          continue;    // not a method or static method
        }
        if (!this->getDerived().HandleFunctionCall(
                fn_decl, underlying_type, static_cast<Expr*>(nullptr)))
          return false;
      }
    }
    // We don't have to simulate a user instantiating the type, because
    // RecursiveASTVisitor.h will recurse on the typedef'ed type for us.
    return true;
  }

  bool TraverseTypedefDecl(TypedefDecl* decl) {
    if (!Base::TraverseTypedefDecl(decl))
      return false;

    return HandleAliasedClassMethods(decl);
  }

  bool TraverseTypeAliasDecl(TypeAliasDecl* decl) {
    if (!Base::TraverseTypeAliasDecl(decl))
      return false;

    return HandleAliasedClassMethods(decl);
  }

  // --- Visitors of types derived from Stmt.

  // Called whenever a variable, function, enum, etc is used.
  bool VisitDeclRefExpr(DeclRefExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;
    // Special case for UsingShadowDecl to track UsingDecls correctly. The
    // actual decl will be reported by obtaining it from the UsingShadowDecl
    // once we've tracked the UsingDecl use.
    if (const UsingShadowDecl* found_decl = DynCastFrom(expr->getFoundDecl())) {
      ReportDeclUse(CurrentLoc(), found_decl);
    } else if (!isa<EnumConstantDecl>(expr->getDecl())) {
      ReportDeclUse(CurrentLoc(), expr->getDecl());
    }
    return Base::VisitDeclRefExpr(expr);
  }

  bool VisitConceptSpecializationExpr(ConceptSpecializationExpr* expr) {
    if (CanIgnoreCurrentASTNode())
      return true;
    ReportDeclUse(CurrentLoc(), expr->getNamedConcept());
    // TODO(bolshakov): analyze type parameter usage.
    return Base::VisitConceptSpecializationExpr(expr);
  }

  // --- Visitors of types derived from Type.

  bool VisitTypedefType(TypedefType* type) {
    if (CanIgnoreCurrentASTNode())
      return true;
    // TypedefType::getDecl() returns the place where the typedef is defined.
    ReportDeclUse(CurrentLoc(), type->getDecl());
    return Base::VisitTypedefType(type);
  }

  bool VisitUsingType(UsingType* type) {
    if (CanIgnoreCurrentASTNode())
      return true;

    if (CanForwardDeclareType(current_ast_node())) {
      ReportDeclForwardDeclareUse(CurrentLoc(), type->getFoundDecl());
    } else {
      ReportDeclUse(CurrentLoc(), type->getFoundDecl());

      // If UsingType refers to a typedef, report the underlying type of that
      // typedef if needed (which is determined in ReportTypeUse).
      const Type* underlying_type = type->getUnderlyingType().getTypePtr();
      if (isa<TypedefType>(underlying_type))
        ReportTypeUse(CurrentLoc(), underlying_type, DerefKind::None);
    }

    return Base::VisitUsingType(type);
  }

  // This is a superclass of RecordType and CXXRecordType.
  bool VisitTagType(TagType* type) {
    if (CanIgnoreCurrentASTNode())
      return true;

    const auto [is_provided, comment] =
        IsProvidedTypeComponent(current_ast_node());
    // If we're forward-declarable, then no complicated checking is
    // needed: just forward-declare.
    if (CanForwardDeclareType(current_ast_node()) && !is_provided) {
      current_ast_node()->set_in_forward_declare_context(true);
      if (compiler()->getLangOpts().CPlusPlus) {
        // In C++, if we're already elaborated ('class Foo x') but not
        // a qualified name ('class ns::Foo x', 'class Class::Nested x') there's
        // no need even to forward-declare.
        // Note that enums are never forward-declarable, so elaborated enums are
        // already short-circuited in CanForwardDeclareType.
        const ASTNode* parent = current_ast_node()->parent();
        if (!IsElaboratedTypeSpecifier(parent) || IsQualifiedNameNode(parent))
          ReportDeclForwardDeclareUse(CurrentLoc(), type->getDecl());
      } else {
        // In C, all struct references are elaborated, so we really never need
        // to forward-declare. But there's one case where an elaborated struct
        // decl in a parameter list causes Clang to warn about constrained
        // visibility, so we recommend forward declaration to avoid the warning.
        // E.g.
        //    void init(struct mystruct* s);
        //      warning: declaration of 'struct mystruct' will not be visible
        //      outside of this function [-Wvisibility]
        if (current_ast_node()->HasAncestorOfType<ParmVarDecl>())
          ReportDeclForwardDeclareUse(CurrentLoc(), type->getDecl());
      }
      return Base::VisitTagType(type);
    }

    // OK, seems to be a use that requires the full type.
    ReportDeclUse(CurrentLoc(), type->getDecl(), comment);
    return Base::VisitTagType(type);
  }

  // Like for CXXConstructExpr, etc., we sometimes need to instantiate
  // a class when looking at TemplateSpecializationType -- for instance,
  // when we need to access a class typedef: MyClass<A>::value_type.
  bool VisitTemplateSpecializationType(TemplateSpecializationType* type) {
    if (CanIgnoreCurrentASTNode())
      return true;

    // If we're not in a forward-declare context, use of a template
    // specialization requires having the full type information.
    if (!CanForwardDeclareType(current_ast_node()) || type->isTypeAlias()) {
      const TemplateInstantiationData data = GetTplInstData(type);
      instantiated_template_visitor_.ScanInstantiatedType(
          current_ast_node(), data.resugar_map, data.provided_types);
    }

    const auto [is_provided, comment] =
        IsProvidedTypeComponent(current_ast_node());
    // Don't call ReportTypeUse here, because scanning of template instantiation
    // internals should be avoided: it is allowed not to provide some of
    // template args.
    if (is_provided || type->isTypeAlias())
      ReportDeclUse(CurrentLoc(), TypeToDeclAsWritten(type), comment);

    return Base::VisitTemplateSpecializationType(type);
  }

  bool VisitElaboratedTypeLoc(ElaboratedTypeLoc type_loc) {
    if (type_loc.getTypePtr()->getKeyword() != ElaboratedTypeKeyword::None) {
      preprocessor_info()
          .FileInfoFor(CurrentFileEntry())
          ->AddElaboratedType(type_loc);
    }
    return Base::VisitElaboratedTypeLoc(type_loc);
  }

  // --- Visitors defined by BaseASTVisitor (not RecursiveASTVisitor).

  bool VisitTemplateName(TemplateName template_name) {
    if (CanIgnoreCurrentASTNode())
      return true;
    if (!Base::VisitTemplateName(template_name))
      return false;
    // We can see TemplateName outside the context of a
    // TemplateSpecializationType when it's either the default argument of a
    // template template arg:
    //    template<template<class T> class A = TplNameWithoutTST> class Foo ...
    // or a deduced template specialization:
    //    std::pair x(10, 20); // type of x is really std::pair<int, int>
    // So that's the only cases we need to handle here.
    // TODO(csilvers): check if this is really forward-declarable or
    // not.  You *could* do something like: 'template<template<class
    // T> class A = Foo> class C { A<int>* x; };' and never
    // dereference x, but that's pretty unlikely.  So for now, we just
    // assume these default template template args always need full
    // type info.
    const ASTNode* ast_node = current_ast_node();
    if (ast_node->ParentIsA<DeducedTemplateSpecializationType>() ||
        IsDefaultTemplateTemplateArg(ast_node)) {
      current_ast_node()->set_in_forward_declare_context(false);
      // Not all template name kinds have an associated template decl; notably
      // dependent names, overloaded template names and ADL-resolved names. Only
      // report the use if we can find a decl.
      if (TemplateDecl* template_decl = template_name.getAsTemplateDecl()) {
        ReportDeclUse(CurrentLoc(), template_decl);
      } else {
        // TODO: There should probably be handling of these delayed-resolution
        // template decls as well, but probably not here.
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
  bool HandleFunctionCall(FunctionDecl* callee, const Type* parent_type,
                          const Expr* calling_expr) {
    if (!Base::HandleFunctionCall(callee, parent_type, calling_expr))
      return false;
    if (!callee || CanIgnoreCurrentASTNode() || CanIgnoreDecl(callee))
      return true;

    if (!IsTemplatizedFunctionDecl(callee) && !IsTemplatizedType(parent_type))
      return true;

    TemplateInstantiationData data = GetTplInstData(callee, calling_expr);

    if (parent_type) {    // means we're a method of a class
      const TemplateInstantiationData class_data = GetTplInstData(parent_type);
      InsertAllInto(class_data.resugar_map, &data.resugar_map);
      InsertAllInto(class_data.provided_types, &data.provided_types);
    }

    if (IsAutocastExpr(current_ast_node())) {
      const set<const Type*> provided_for_autocast =
          GetProvidedTypesForAutocast(current_ast_node());
      data.provided_types.insert(provided_for_autocast.begin(),
                                 provided_for_autocast.end());
    }
    instantiated_template_visitor_.ScanInstantiatedFunction(
        callee, parent_type, current_ast_node(), data.resugar_map,
        data.provided_types);
    return true;
  }

  // --- Handler declared in IwyuBaseASTVisitor.

  void ReportTplSpecComponentTypes(const TemplateSpecializationType* type,
                                   const set<const Type*>& blocked_types) {
    TemplateInstantiationData data = GetTplInstData(type);
    ASTNode node(type);
    node.SetParent(current_ast_node());
    data.provided_types.insert(blocked_types.begin(), blocked_types.end());
    instantiated_template_visitor_.ScanInstantiatedType(&node, data.resugar_map,
                                                        data.provided_types);
  }

  set<const Type*> GetProvidedByTplArg(const Type* type) {
    set<const Type*> res;
    if (!type)
      return res;
    if (const auto* elaborated = dyn_cast<ElaboratedType>(type)) {
      const NestedNameSpecifier* nns = elaborated->getQualifier();
      while (nns) {
        if (const Type* host = nns->getAsType())
          InsertAllInto(GetTplInstData(host).provided_types, &res);
        nns = nns->getPrefix();
      }
    }
    if (const auto* tpl_spec = type->getAs<TemplateSpecializationType>()) {
      if (tpl_spec->isTypeAlias())
        InsertAllInto(GetTplInstData(tpl_spec).provided_types, &res);
    }
    return res;
  }

  bool SourceOrTargetTypeIsProvided(const ASTNode* construct_expr_node) const {
    const auto* expr = construct_expr_node->GetAs<CXXConstructExpr>();
    CHECK_(expr);
    const Type* type = GetCanonicalType(GetTypeOf(expr));
    if (expr->getConstructor()->isCopyOrMoveConstructor()) {
      const Type* source_type =
          GetTypeOf(expr->getArg(0)->IgnoreParenImpCasts());
      if (GetProvidedTypeComponents(source_type).count(type))
        return true;
    }

    auto get_call_expr = [](const ASTNode* node) -> const CallExpr* {
      if (const auto* call_expr = node->template GetParentAs<CallExpr>())
        return call_expr;
      if (node->template ParentIsA<CXXBindTemporaryExpr>())
        return node->template GetAncestorAs<CallExpr>(2);
      return nullptr;
    };

    if (const CallExpr* call_expr = get_call_expr(construct_expr_node)) {
      // There is no callee for unresolved call exprs in template definitions.
      if (const FunctionDecl* fn_decl = call_expr->getDirectCallee()) {
        // For nontemplated functions, 'CXXConstructExpr' type appears to be
        // sugared and hence handled correctly inside 'ReportTypeUse'.
        if (GetTplInstData(fn_decl, call_expr).provided_types.count(type))
          return true;
      }
    }
    return false;
  }

 private:
  set<const Type*> GetProvidedTypeComponents(const Type* type) const {
    const Type* desugared_until_typedef = Desugar(type);
    if (const auto* typedef_type =
            dyn_cast_or_null<TypedefType>(desugared_until_typedef)) {
      const TypedefNameDecl* decl = typedef_type->getDecl();
      return GetProvidedTypesForAlias(decl->getUnderlyingType().getTypePtr(),
                                      GetLocation(decl));
    }
    // TODO(bolshakov): handle alias templates.
    return set<const Type*>();
  }

  TemplateInstantiationData GetTplInstData(const Type* type) const {
    return GetTplInstDataForClass(type, [this](const Type* type) {
      return GetProvidedTypeComponents(type);
    });
  }

  TemplateInstantiationData GetTplInstData(const FunctionDecl* decl,
                                           const Expr* calling_expr) const {
    return GetTplInstDataForFunction(
        decl, calling_expr,
        [this](const Type* type) { return GetProvidedTypeComponents(type); });
  }

  pair<bool, const char*> CanBeProvidedTypeComponent(
      const ASTNode* node) const {
    if (node->HasAncestorOfType<TypedefNameDecl>() ||
        node->HasAncestorOfType<TemplateTypeParmDecl>()) {
      return pair(true, nullptr);
    }

    if (const FunctionDecl* decl = node->GetAncestorAs<FunctionDecl>()) {
      // No point in author-intent analysis of function definitions
      // in source files, or for builtins, or for friend declarations.
      if (!IsInHeader(decl) || IsFriendDecl(decl))
        return pair(false, nullptr);
      for (const ParmVarDecl* param : decl->parameters()) {
        if (node->StackContainsContent(param)) {
          if (HasImplicitConversionConstructor(GetTypeOf(param)))
            return pair(true, "(for autocast)");
          return pair(false, nullptr);
        }
      }
      const Type* return_type = decl->getReturnType().getTypePtr();
      if (node->StackContainsContent(return_type) &&
          !decl->isThisDeclarationADefinition() &&
          !IsPointerOrReferenceAsWritten(return_type)) {
        return pair(true, "(for fn return type)");
      }
    }
    return pair(false, nullptr);
  }

  // The function is used to determine if the full type info is needed
  // at a function or type alias declaration side due to author's decision
  // to provide it.
  pair<bool, const char*> IsProvidedTypeComponent(const ASTNode* node) const {
    // A function or type alias author can choose whether to provide a type
    // or not only when the type can be forward declared.
    if (!CanForwardDeclareType(node))
      return pair(false, nullptr);
    const auto [can_be_provided, comment] = CanBeProvidedTypeComponent(node);
    if (can_be_provided && !CodeAuthorWantsJustAForwardDeclare(
                               node->GetAs<Type>(), node->GetLocation())) {
      return pair(true, comment);
    }
    return pair(false, nullptr);
  }

  // Class we call to handle instantiated template functions and classes.
  InstantiatedTemplateVisitor instantiated_template_visitor_;
};  // class IwyuAstConsumer

// We use an ASTFrontendAction to hook up IWYU with Clang.
class IwyuAction : public ASTFrontendAction {
 public:
  IwyuAction() = delete;

  explicit IwyuAction(const ToolChain& toolchain) : toolchain(toolchain) {
  }

 protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(
      CompilerInstance& compiler,  // NOLINT
      llvm::StringRef /* dummy */) override {
    // Do this first thing after getting our hands on initialized
    // CompilerInstance and ToolChain objects.
    InitGlobals(compiler, toolchain);

    Preprocessor& preprocessor = compiler.getPreprocessor();
    auto* const preprocessor_consumer = new IwyuPreprocessorInfo(preprocessor);
    preprocessor.addPPCallbacks(
        std::unique_ptr<PPCallbacks>(preprocessor_consumer));
    preprocessor.addCommentHandler(preprocessor_consumer);

    auto* const visitor_state =
        new VisitorState(&compiler, *preprocessor_consumer);
    return std::unique_ptr<IwyuAstConsumer>(new IwyuAstConsumer(visitor_state));
  }

 private:
  // ToolChain is not copyable, but it's owned by Compilation which has the same
  // lifetime as CompilerInstance, so it should be alive for as long as we are.
  const ToolChain& toolchain;
};

} // namespace include_what_you_use

int main(int argc, char **argv) {
  using clang::driver::ToolChain;
  using include_what_you_use::ExecuteAction;
  using include_what_you_use::IwyuAction;
  using include_what_you_use::OptionsParser;

  llvm::llvm_shutdown_obj scoped_shutdown;

  // X86 target is required to parse Microsoft inline assembly, so we hope it's
  // part of all targets. Clang parser will complain otherwise.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();

  // The command line should look like
  //   path/to/iwyu -Xiwyu --verbose=4 [-Xiwyu --other_iwyu_flag]... \
  //       CLANG_FLAGS... foo.cc
  OptionsParser options_parser(argc, argv);
  if (!ExecuteAction(options_parser.clang_argc(), options_parser.clang_argv(),
                     [](const ToolChain& toolchain) {
                       return std::make_unique<IwyuAction>(toolchain);
                     })) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
