//===--- lateparsed_template-notchecked.h - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_LATEPARSED_NOTCHECKED_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_LATEPARSED_NOTCHECKED_H_

// A crazy late-parsed function template.
// IWYU should not attempt to parse it, since it's not in a file signed up for
// analysis.
template<class T>
void unchecked() {
  this is late parsed, and makes no sense;
};

// Here's a symbol we reference from the main test file.
const int kUsableSymbol = 100;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_LATEPARSED_NOTCHECKED_H_
