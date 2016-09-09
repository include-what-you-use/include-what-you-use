//===--- badinc-private.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a 'private' header file re-exported by badinc-inl.h
// (the relevant mapping is in iwyu_include_picker.cc).

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_PRIVATE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_PRIVATE_H_

enum HPrivate_Enum { HP1, HP2, HP3 };

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_PRIVATE_H_
