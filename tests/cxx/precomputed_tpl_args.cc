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

#include <vector>
#include <set>
#include <map>
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


/**** IWYU_SUMMARY

tests/cxx/precomputed_tpl_args.cc should add these lines:
#include "tests/cxx/precomputed_tpl_args-i1.h"

tests/cxx/precomputed_tpl_args.cc should remove these lines:

The full include-list for tests/cxx/precomputed_tpl_args.cc:
#include <bitset>  // for bitset
#include <map>  // for map
#include <set>  // for set
#include <vector>  // for vector
#include "tests/cxx/precomputed_tpl_args-d1.h"  // for D1SpecializationClass, less
#include "tests/cxx/precomputed_tpl_args-i1.h"  // for IndirectClass, SpecializationClass, less

***** IWYU_SUMMARY */
