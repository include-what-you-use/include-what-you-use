//===--- iterator.cc - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// We have special-case code to deal with foo::iterator, for various
// STL classes foo.  They are complicated because they often map to
// __normal_iterator<foo>, and we have to map back.  There are also
// issues with reverse_iterator.
//
// Basically, we don't want any of the code below to result in an
// #include of <iterator>

#include <algorithm>
#include <vector>
#include <list>

std::vector<int> vi;
std::vector<int>::iterator vi_begin = vi.begin();
std::vector<int>::const_iterator vi_cbegin = vi.begin();
std::vector<int>::reverse_iterator vi_rbegin = vi.rbegin();
std::vector<int>::const_reverse_iterator vi_crbegin = vi.rbegin();
void VectorFns() {
  // Tricky issue with deduced template args.
  std::find(vi_begin, vi_begin, 5);
  // Issues with operator!=, operator bool, etc.
  for (std::vector<int>::iterator it = vi_begin; it != vi_begin; ++it) ;
  for (std::vector<int>::reverse_iterator it = vi_rbegin; it != vi_rbegin; ++it)
    ;
}

std::list<int> li;
std::list<int>::iterator li_begin = li.begin();
std::list<int>::const_iterator li_cbegin = li.begin();
std::list<int>::reverse_iterator li_rbegin = li.rbegin();
std::list<int>::const_reverse_iterator li_crbegin = li.rbegin();
void ListFns() {
  std::find(li_begin, li_begin, 5);
  for (std::list<int>::iterator it = li_begin; it != li_begin; ++it) ;
  for (std::list<int>::reverse_iterator it = li_rbegin; it != li_rbegin; ++it) ;
}

/**** IWYU_SUMMARY

(tests/iterator.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
