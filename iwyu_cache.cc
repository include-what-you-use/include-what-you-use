//===--- iwyu_cache.cpp - cache of AST-derived information, for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <set>
#include <string>
#include "iwyu_cache.h"
#include "iwyu_ast_util.h"
#include "iwyu_location_util.h"
#include "iwyu_preprocessor.h"
#include "iwyu_stl_util.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Type.h"

using clang::ClassTemplateSpecializationDecl;
using clang::Type;
using clang::TemplateArgument;
using clang::TemplateArgumentList;
using clang::TemplateSpecializationType;
using std::set;
using std::string;

namespace include_what_you_use {

// These are types that all have the same property: instantiating
// the type requires full use of all explicitly-listed template
// arguments, and full use of all default templated arguments that
// the class does not intend-to-provide.  (So vector<MyClass>
// required full use of MyClass, but not of allocator<MyClass>).
// TODO(csilvers): put this somewhere easier to modify, and add to it.
static const char* const kFullUseTypes[] = {
  "__gnu_cxx::hash_map",
  "__gnu_cxx::hash_multimap",
  "__gnu_cxx::hash_multiset",
  "__gnu_cxx::hash_set",
  "std::bitset",
  "std::deque",
  "std::list",
  "std::map",
  "std::multimap",
  "std::multiset",
  "std::set",
  "std::slist",
  "std::vector",
};

// If the passed-in tpl_decl is one of the classes we have hard-coded
// the full-use type information for, populate the cache with the
// appropriate full-use type information for the given instantiation.
// Note that we only populate the cache for class member uses, not
// function calls -- that is, we can use the hard-coded data to say
// full use-info for 'sizeof(vector<MyClass>)', but not for
// 'myclass_vector.clear();'.  This is because the former never tries
// to instantiate methods, making the hard-coding much easier.
set<const Type*> FullUseCache::GetPrecomputedUnsugaredFullUseTypes(
    const IwyuPreprocessorInfo& preprocessor_info,
    const TemplateSpecializationType* tpl_type) {
  static const int fulluse_size = (sizeof(kFullUseTypes) /
                                   sizeof(*kFullUseTypes));
  static const set<string> fulluse_types(kFullUseTypes,
                                         kFullUseTypes + fulluse_size);

  // The decl for this type is desugared, which means it includes
  // default template arguments.  (Sadly, it also resolves
  // typedefs/etc, which we'll have to undo.)
  const ClassTemplateSpecializationDecl* tpl_decl
      = DynCastFrom(TypeToDeclAsWritten(tpl_type));
  CHECK_(tpl_decl && "tpl-type decl is not a tpl specialization?");
  if (!include_what_you_use::Contains(
          fulluse_types, tpl_decl->getQualifiedNameAsString()))
    return set<const Type*>();

  VERRS(6) << "(Using pre-computed list of full-use information for "
           << tpl_decl->getQualifiedNameAsString() << ")\n";
  // The code below doesn't handle template-template args/etc.  None
  // of the types in kFullUseTypes should have those.  Just verify.
  const TemplateArgumentList& all_tpl_args = tpl_decl->getTemplateArgs();
  for (unsigned i = 0; i < all_tpl_args.size(); ++i) {
    CHECK_((all_tpl_args.get(i).getKind() == TemplateArgument::Type)
           && "kFullUseType types must contain only 'type' template args");
  }

  // First, collect all explicit template args.  (Note: we'll have to
  // do something more clever here if any types in kFullUseTypes
  // start accepting template-template types.)
  set<const Type*> retval = GetExplicitTplTypeArgsOf(tpl_type);

  // Now, collect all default template args (after the explicit
  // args).  We collect those that our type does not 'intend to
  // provide'.
  for (unsigned i = tpl_type->getNumArgs(); i < all_tpl_args.size(); ++i) {
    const Type* arg_type = all_tpl_args.get(i).getAsType().getTypePtr();
    if (preprocessor_info.PublicHeaderIntendsToProvide(
            GetFileEntry(tpl_decl),
            GetFileEntry(TypeToDeclAsWritten(arg_type))))
      continue;
    retval.insert(arg_type);
  }
  return retval;
}

}  // namespace include_what_you_use
