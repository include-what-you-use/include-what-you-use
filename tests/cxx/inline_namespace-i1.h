//===--- inline_namespace-i1.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_INLINE_NAMESPACE_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_INLINE_NAMESPACE_I1_H_

namespace xyz {
inline namespace v1 {

struct Foo {
  int value;
};

}  // namespace v1
}  // namespace xyz

#endif // INCLUDE_WHAT_YOU_USE_TESTS_CXX_INLINE_NAMESPACE_I1_H_

