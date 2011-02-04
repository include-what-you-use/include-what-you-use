//===--- iwyu_cache.h - cache of AST-derived information, for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file holds caches for information that clang might have to use
// many times, but is expensive to compute.  For now, the only cache
// is the 'instantiation cache': when instantiating a template, what
// methods are called, and what template arguments are fully used?

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_CACHE_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_CACHE_H_

#include <map>
#include <set>
#include <string>
#include <utility>  // for pair
#include "iwyu_stl_util.h"

namespace clang {
class Type;
class NamedDecl;
class TemplateSpecializationType;
}

namespace include_what_you_use {

using std::map;
using std::pair;
using std::set;
using std::string;


class IwyuPreprocessorInfo;

// This cache is used to store 'full use information' for a given
// templated function call or type instantiation:
// 1) If you call MyClass<Foo, Bar>::baz(), what template arguments
//    do you need the full type information for?  (Might be Foo, Bar,
//    both, or neither, depending on what baz() does.)
// 2) If you do 'sizeof(MyClass<Foo, Bar>)', what template arguments
//    do you need the full type information for?  (Might be Foo,
//    Bar, both, or neither, depending on what fields MyClass has.)
// 3) If you call MyClass<Foo, Bar>::baz(), what methods are called
//    indirectly?  (ie baz() might call another method MyClass::bar(),
//    or OtherClass::foo().)  This is used to detect cases where we
//    call a method that is written in a different file from the
//    class it belongs to -- we will need to #include the file with
//    that method in it.  The method called may well depend on the
//    template arguments, hence the need to instantiate.
// For each of these, the answer is always the same for the given
// Decl (function call or class instantiation) with the same template
// args.  So we store this info in a cache, since it's very expensive
// to compute.

class FullUseCache {
 public:
  // The first part of the key is the decl or type that we're
  // caching reporting-info for.  Since what we report depends on
  // what the types-of-interest were, we store that in the key too.
  typedef pair<const void*,
               set<const clang::Type*> > Key;
  // The value are the types and decls we reported.
  typedef pair<const set<const clang::Type*>,
               const set<const clang::NamedDecl*> > Value;

  void Insert(const void* decl_or_type,
              const set<const clang::Type*>& types_of_interest,
              const set<const clang::Type*>& reported_types,
              const set<const clang::NamedDecl*>& reported_decls) {
    // TODO(csilvers): should in_forward_declare_context() be in Key too?
    cache_.insert(pair<Key,Value>(Key(decl_or_type, types_of_interest),
                                  Value(reported_types, reported_decls)));
  }

  // types_of_interest are the template arguments used to instantiate
  // this template.
  bool Contains(const void* key,
                const set<const clang::Type*>& types_of_interest) const {
    return include_what_you_use::Contains(
        cache_, Key(key, types_of_interest));
  }

  // You must call Contains() before calling these, to make sure the
  // key is in the cache.
  const set<const clang::Type*>& GetFullUseTypes(
      const void* key, const set<const clang::Type*>& types_of_interest) const {
    const Value* value = FindInMap(&cache_, Key(key, types_of_interest));
    assert(value && "Must call Contains() before calling GetFullUseTypes()");
    return value->first;
  }

  const set<const clang::NamedDecl*>& GetFullUseDecls(
      const void* key, const set<const clang::Type*>& types_of_interest) const {
    const Value* value = FindInMap(&cache_, Key(key, types_of_interest));
    assert(value && "Must call Contains() before calling GetFullUseDecls()");
    return value->second;
  }

  // In addition to the normal cache, which is filled via Insert()
  // calls, we also have a special, hard-coded cache holding full-use
  // type information for common STL types.  Note that since we only
  // have full-use type information, and not full-use decl
  // information, this cache is only appropriate when instantiating a
  // type ('sizeof(vector<MyClass>)'), not when making a function call
  // ('myclass_vector->clear()').
  //    NOTE: because this cache is hard-coded, the types may not be
  // sugared properly: the output might be 'MyUnderlyingType' when the
  // input is 'vector<MyTypedef>'.  You will have to resugar yourself.
  // That is why this is implemented in a different function, and not
  // available via GetFullUseType(), which does not have this problem
  // with sugaring.
  static set<const clang::Type*> GetPrecomputedUnsugaredFullUseTypes(
      const IwyuPreprocessorInfo& preprocessor_info,
      const clang::TemplateSpecializationType* tpl_type);

 private:
  map<Key, Value> cache_;
};

// This class allows us to update multiple cache entries at once.
// For instance, suppose A<Foo, Bar>() calls B<Foo, Bar>(), which
// requires the full type info for Foo.  Then we want to add a cache
// entry (B, Foo) ("B requires the full type info for Foo"), but we
// also want to add a cache entry (A, Foo) ("A requires the full
// type info for Foo", due to its calling B).  The way we do this is
// whenever we enter a function -- or instantiate a type -- we add
// it to our set of 'currently active functions', cache_storers_.
// Whenever we decide we need full type info for some type Foo, we
// add a new cache entry for every function/type in cache_storers_.
class CacheStoringScope {
 public:
  CacheStoringScope(set<CacheStoringScope*>* cache_storers,
                    FullUseCache* cache,
                    const void* key,
                    const set<const clang::Type*>& types_of_interest)
      : cache_storers_(cache_storers), cache_(cache),
        key_(key), types_of_interest_(types_of_interest) {
    // Register ourselves so ReportDeclUse() and ReportTypeUse()
    // will call back to us.
    cache_storers_->insert(this);
  }

  ~CacheStoringScope() {
    cache_->Insert(key_, types_of_interest_,
                   reported_types_, reported_decls_);
    cache_storers_->erase(this);
  }

  // These are what ReportDeclUse() and ReportTypeUse() call to
  // populate this cache entry.
  void NoteReportedType(const clang::Type* type) {
    reported_types_.insert(type);
  }
  void NoteReportedDecl(const clang::NamedDecl* decl) {
    reported_decls_.insert(decl);
  }

 private:
  set<CacheStoringScope*>* const cache_storers_;
  FullUseCache* const cache_;
  const void* const key_;
  const set<const clang::Type*> types_of_interest_;
  set<const clang::Type*> reported_types_;
  set<const clang::NamedDecl*> reported_decls_;
};

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_CACHE_H_
