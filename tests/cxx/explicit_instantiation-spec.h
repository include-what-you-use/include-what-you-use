//===--- explicit_instantiation-spec.h - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/explicit_instantiation-template_direct.h"

template <typename T>
class Template;

template <typename T>
class Template<T*> {};

/**** IWYU_SUMMARY

tests/cxx/explicit_instantiation-spec.h should add these lines:

tests/cxx/explicit_instantiation-spec.h should remove these lines:
- #include "tests/cxx/explicit_instantiation-template_direct.h"  // lines XX-XX

The full include-list for tests/cxx/explicit_instantiation-spec.h:
template <typename T> class Template;  // lines XX-XX+1

***** IWYU_SUMMARY */
