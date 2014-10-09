//===--- no_deque.cc - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A very small test that makes sure we don't suggest deque when using
// a queue.

#include <queue>

int main() {
  std::queue<int> q;
  (void)q;
}

/**** IWYU_SUMMARY

(tests/cxx/no_deque.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
