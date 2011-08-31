//===--- iwyu_cache.cpp - cache of AST-derived information, for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_cache.h"

#include <set>
#include <string>

#include "iwyu_ast_util.h"
#include "iwyu_stl_util.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/Type.h"

using clang::ClassTemplateSpecializationDecl;
using clang::NamedDecl;
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
map<const Type*, const Type*> FullUseCache::GetPrecomputedResugarMap(
    const TemplateSpecializationType* tpl_type) {
  static const int fulluse_size = (sizeof(kFullUseTypes) /
                                   sizeof(*kFullUseTypes));
  static const set<string> fulluse_types(kFullUseTypes,
                                         kFullUseTypes + fulluse_size);

  const NamedDecl* tpl_decl = TypeToDeclAsWritten(tpl_type);
  if (!ContainsKey(fulluse_types, tpl_decl->getQualifiedNameAsString()))
    return map<const Type*, const Type*>();

  // The code below doesn't handle template-template args/etc.  None
  // of the types in kFullUseTypes should have those.  Just verify,
  // when we can.
  if (const ClassTemplateSpecializationDecl* tpl_spec_decl
      = DynCastFrom(tpl_decl)) {
    const TemplateArgumentList& all_tpl_args = tpl_spec_decl->getTemplateArgs();
    for (unsigned i = 0; i < all_tpl_args.size(); ++i) {
      CHECK_((all_tpl_args.get(i).getKind() == TemplateArgument::Type)
             && "kFullUseType types must contain only 'type' template args");
    }
  }

  // The default resugar-map works correctly for all these types (by
  // design): we fully use all template types.  (Note: we'll have to
  // do something more clever here if any types in kFullUseTypes start
  // accepting template-template types.)
  return GetTplTypeResugarMapForClassNoComponentTypes(tpl_type);
}

}  // namespace include_what_you_use
