//===--- iwyu_test_helpers.h - iwyu unittest helpers --------*- C++ -*-----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cstddef>
#include <string>
#include <vector>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/raw_ostream.h"

namespace include_what_you_use {
using llvm::ArrayRef;
using llvm::raw_string_ostream;
using std::string;
using std::vector;

// Returns a string representing the first element where the arrays actual and
// expected differ, or "" if they're identical. The arrays can be different
// types as long as E is implicitly comparable to A (e.g. const char* vs.
// std::string). All element types must be streamable to raw_ostream.
template <class E, class A>
string ArrayDiff(ArrayRef<E> expected, ArrayRef<A> actual) {
  string diff;
  raw_string_ostream out(diff);

  // Not interested in paying for including <algorithm> for std::min.
  size_t min_size =
      (actual.size() < expected.size()) ? actual.size() : expected.size();
  for (size_t i = 0; i < min_size; ++i) {
    if (expected[i] != actual[i]) {
      out << "Differ at #" << i << ": expected=" << expected[i]
          << ", actual=" << actual[i];
      return diff;
    }
  }
  if (expected.size() < actual.size()) {
    out << "Differ at #" << expected.size()
        << ": expected at EOF, actual=" << actual[expected.size()];
    return diff;
  } else if (actual.size() < expected.size()) {
    out << "Differ at #" << expected.size()
        << ": expected=" << expected[actual.size()] << ", actual at EOF";
    return diff;
  } else {
    return "";  // They're equal.
  }
}

// Partial specializations to allow for various useful spellings of the
// expected, not an exhaustive combination of all possible types.

// ArrayDiff({"a", "b"}, vec)
template <class E, class A, size_t N>
string ArrayDiff(const E (&expected)[N], const vector<A>& actual) {
  return ArrayDiff(ArrayRef<E>(expected), ArrayRef<A>(actual));
}

}  // namespace include_what_you_use
