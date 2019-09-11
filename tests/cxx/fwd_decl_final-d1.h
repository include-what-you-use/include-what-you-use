//===--- fwd_decl_final-d1.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_FWD_DECL_FINAL_D1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_FWD_DECL_FINAL_D1_H_


template <typename T>
class FinalTemplate final {};

class FinalClass final {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_FWD_DECL_FINAL_D1_H_
