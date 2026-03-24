//===--- 1803.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++20
// IWYU_XFAIL

#include "funcs.h"

void Fn() {
  // IWYU reports std::get as part of the structured binding initialization.
  auto [i1, i2] = GetPair();
  for (auto [j1, j2] : GetVectorOfPairs())
    ;
}

/**** IWYU_SUMMARY

tests/bugs/1803/1803.cc should add these lines:
#include <utility>
#include <vector>

tests/bugs/1803/1803.cc should remove these lines:

The full include-list for tests/bugs/1803/1803.cc:
#include <utility>  // for pair
#include <vector>  // for vector
#include "funcs.h"  // for GetPair, GetVectorOfPairs

***** IWYU_SUMMARY */
