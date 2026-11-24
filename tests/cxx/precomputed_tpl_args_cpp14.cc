//===--- precomputed_tpl_args_cpp14.cc - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . --std=c++14

// Tests full contained type requirements for STL containers in C++14.
// Full contained type info is required for an instantiation according
// to the standard even if no one member function is called.

#include <cstddef>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "tests/cxx/direct.h"

// Unordered containers need a hash function that can be invoked on the key
// type, and std::hash<IndirectClass> is disabled. Newer standard libraries
// (e.g. libstdc++ 15) check this when the container class is instantiated.
struct AnyHash {
  template <typename T>
  std::size_t operator()(const T&) const {
    return 0;
  }
};

void Fn() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::vector<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::deque<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::list<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::forward_list<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::set<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::multiset<IndirectClass>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::map<IndirectClass, int>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::multimap<IndirectClass, int>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::unordered_set<IndirectClass, AnyHash>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::unordered_multiset<IndirectClass, AnyHash>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::unordered_map<IndirectClass, int, AnyHash>);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(std::unordered_multimap<IndirectClass, int, AnyHash>);
}

/**** IWYU_SUMMARY

tests/cxx/precomputed_tpl_args_cpp14.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/precomputed_tpl_args_cpp14.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/precomputed_tpl_args_cpp14.cc:
#include <cstddef>  // for size_t
#include <deque>  // for deque
#include <forward_list>  // for forward_list
#include <list>  // for list
#include <map>  // for map, multimap
#include <set>  // for multiset, set
#include <unordered_map>  // for unordered_map, unordered_multimap
#include <unordered_set>  // for unordered_multiset, unordered_set
#include <vector>  // for vector
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
