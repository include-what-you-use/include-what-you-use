//===--- fwd_decl_print-i1.h - test input file for iwyu -- C++ ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_FWD_DECL_PRINT_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_FWD_DECL_PRINT_I1_H_

template<typename T>
concept Addable = requires (T x) { x + x; };

#endif
