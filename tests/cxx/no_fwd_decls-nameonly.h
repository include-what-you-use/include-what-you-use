//===--- no_fwd_decls-nameonly.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_NO_FWD_DECLS_NAMEONLY_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_NO_FWD_DECLS_NAMEONLY_H_

class NameOnly;

const void* AddressOf(const NameOnly& a) {
  return reinterpret_cast<const void*>(&a);
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_NO_FWD_DECLS-NAMEONLY_H_
