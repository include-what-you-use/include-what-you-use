//===--- badinc-private2.h - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a 'private' header file re-exported by badinc-inl.h
// (the relevant mapping is in iwyu_include_picker.cc),
// that badinc-inl.h never uses any symbols from.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_PRIVATE2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_PRIVATE2_H_

#define UNUSED_BADINC_PRIVATE2_MACRO  true

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_PRIVATE2_H_
