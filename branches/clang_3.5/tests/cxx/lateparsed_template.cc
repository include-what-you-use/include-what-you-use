//===--- lateparsed_template.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests IWYU handling of function templates when used with the MSVC-compatible
// -fdelayed-template-parsing flag.
//
// IWYU should eagerly parse templates before analysis, to make sure we account
// for symbols in template bodies.
//
// This test is only meaningful when run with the -fdelayed-template-parsing
// flag.

#include "tests/cxx/direct.h"
#include "tests/cxx/lateparsed_template-notchecked.h"

// With -fdelayed-template-parsing, a function template is only parsed once
// it's instantiated (and it never is). IWYU has a special pass to force
// the template parsing necessary to discover the use of IndirectClass.
template <typename T> void function_template() {
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass ic;
}

// Methods of class templates are also late parsed...
template <typename T> class ClassTemplate {
  void method() {
    // IWYU: IndirectClass is...*indirect.h
    IndirectClass ic;
  }
};

// ... as are member function templates of plain classes.
class Class {
  template <typename T> void method_template() {
    // IWYU: IndirectClass is...*indirect.h
    IndirectClass ic;
  }
};

const int kConstUse = kUsableSymbol;

/**** IWYU_SUMMARY

tests/cxx/lateparsed_template.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/lateparsed_template.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/lateparsed_template.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/lateparsed_template-notchecked.h"  // for kUsableSymbol

***** IWYU_SUMMARY */
