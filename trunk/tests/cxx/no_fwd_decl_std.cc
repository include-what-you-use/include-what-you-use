//===--- no_fwd_decl_std.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Do not forward-declare anything from the std namespace.  (This is a
// matter of policy, not language correctness.)  When testing, we have
// to make sure to pick something that wouldn't be caught under
// another rule (such as the 'never forward-declare anything with
// default template arguments rule).

#include <utility>

std::pair<int, int>* p1 = 0;

// Make sure it the rule works even with a using declaration
using std::pair;

pair<float, float>* p2 = 0;

// We should remove this fwd-decl, since we're keeping utility
namespace std {
template<typename T, typename U> struct pair;
}

pair<char, char>* p3 = 0;


/**** IWYU_SUMMARY

tests/cxx/no_fwd_decl_std.cc should add these lines:

tests/cxx/no_fwd_decl_std.cc should remove these lines:
- namespace std { template <typename T, typename U> struct pair; }  // lines XX-XX

The full include-list for tests/cxx/no_fwd_decl_std.cc:
#include <utility>  // for pair

***** IWYU_SUMMARY */
