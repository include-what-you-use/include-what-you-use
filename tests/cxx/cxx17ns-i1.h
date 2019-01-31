//===--- cxx17ns-i1.h - test input file for iwyu --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_CXX17NS_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_CXX17NS_I1_H_

namespace a {
namespace b {
namespace c {
struct One {
  One();
};
}  // namespace c
struct One2 {
  One2();
};
}  // namespace b
struct One3 {
  One3();
};
namespace {
struct One4 {
  One4();
};
}
}  // namespace a

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_CXX17NS_I1_H_
