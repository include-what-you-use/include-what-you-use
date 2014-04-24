//===--- prefix_header_operator_new.cc - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that IWYU can tell if made-up, not encountered header is prefix header.
// The main difference between the current test and prefix_header_attribution.cc
// is that in this test <new> is included neither from source nor from command
// line so that header <new> isn't encountered.

template<typename T> void CallPlacementNew(T *t) {
  // IWYU: operator new is...*<new>
  new (t) T();
}

/**** IWYU_SUMMARY

tests/cxx/prefix_header_operator_new.cc should add these lines:
#include <new>

tests/cxx/prefix_header_operator_new.cc should remove these lines:

The full include-list for tests/cxx/prefix_header_operator_new.cc:
#include <new>  // for operator new

***** IWYU_SUMMARY */
