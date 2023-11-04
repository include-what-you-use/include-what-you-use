//===--- precomputed_tpl_args.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests the precomputed template-arg-use list in iwyu_cache.cc.

#include <forward_list>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include "tests/cxx/precomputed_tpl_args-d1.h"

template <typename T> struct Identity {
  T t;
};

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
std::vector<IndirectClass> ic_vec;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
std::vector<Identity<IndirectClass> > i_ic_vec;

// IWYU: IndirectClass needs a declaration
std::vector<IndirectClass*> icptr_vec;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
std::set<IndirectClass> ic_set;

// This class provides a specialization of less that we should see.
// IWYU: SpecializationClass needs a declaration
// IWYU: SpecializationClass is...*precomputed_tpl_args-i1.h
// IWYU: std::less is...*precomputed_tpl_args-i1.h
std::set<SpecializationClass> sc_set;

// This class provides a specialization of less that we should see,
// in d1.h.  We should be sure not to remove -d1.h as a result!
std::set<D1SpecializationClass> d1sc_set;

// We were seeing a bug where, inside a typedef, we got an incorrect
// iwyu violation report for less<>.  Make sure that's fixed.
// (This should be a new type from above so we don't hit the cache.)
typedef std::set<int> IntSet;
// Call the constructor to cause it to be instantiated, so iwyu visits it.
IntSet int_set;

// bitset should not be precomputed, since it has a non-class arg.
std::bitset<5> bitset;


// When considering a precomputed type (like map<>) inside a templated
// class, make sure that we only consider the precomputed args that
// are part of the instantiation of the templated class.  That is,
// for map<T, SpecializationClass>, we should only consider T.

template<typename T> class TemplatedClass {
  // IWYU: SpecializationClass is...*precomputed_tpl_args-i1.h
  // IWYU: SpecializationClass needs a declaration
  std::map<SpecializationClass, T> t1;
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  // IWYU: IndirectClass needs a declaration
  std::map<T, IndirectClass> t3;
};

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
TemplatedClass<IndirectClass> tc_ic;

// TODO(csilvers): IWYU: std::less is...*precomputed_tpl_args-i1.h
// IWYU: SpecializationClass needs a declaration
// IWYU: SpecializationClass is...*precomputed_tpl_args-i1.h
TemplatedClass<SpecializationClass> tc_sc;

void Fn() {
  // Since C++17, full contained type isn't needed for std::vector, std::list,
  // and std::forward_list instantiations.
  // IWYU: IndirectClass needs a declaration
  (void)sizeof(std::vector<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  (void)sizeof(std::list<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  (void)sizeof(std::forward_list<IndirectClass>);

  // Full type is still needed for the other standard containers.
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  (void)sizeof(std::set<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  (void)sizeof(std::multiset<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  (void)sizeof(std::map<IndirectClass, int>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  (void)sizeof(std::multimap<IndirectClass, int>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  (void)sizeof(std::unordered_set<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  (void)sizeof(std::unordered_multiset<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  (void)sizeof(std::unordered_map<IndirectClass, int>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  (void)sizeof(std::unordered_multimap<IndirectClass, int>);

  // Full type is required for, e.g., destroying sequence containers.
  // IWYU: IndirectClass needs a declaration
  std::list<IndirectClass>* l = nullptr;
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  delete l;
  // IWYU: IndirectClass needs a declaration
  std::forward_list<IndirectClass>* fl = nullptr;
  // IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
  delete fl;
}

/**** IWYU_SUMMARY

tests/cxx/precomputed_tpl_args.cc should add these lines:
#include "tests/cxx/precomputed_tpl_args-i1.h"

tests/cxx/precomputed_tpl_args.cc should remove these lines:

The full include-list for tests/cxx/precomputed_tpl_args.cc:
#include <bitset>  // for bitset
#include <forward_list>  // for forward_list
#include <list>  // for list
#include <map>  // for map, multimap
#include <set>  // for multiset, set
#include <unordered_map>  // for unordered_map, unordered_multimap
#include <unordered_set>  // for unordered_multiset, unordered_set
#include <vector>  // for vector
#include "tests/cxx/precomputed_tpl_args-d1.h"  // for D1SpecializationClass, less
#include "tests/cxx/precomputed_tpl_args-i1.h"  // for IndirectClass, SpecializationClass, less

***** IWYU_SUMMARY */
