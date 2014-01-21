//===--- forward_declare_in_macro.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we forward-declare in a macro, that the line
// numbers that iwyu emits about where the forward-declare lives in
// the code, use the macro-instantiation location, never the
// macro-spelling location.  In this case, the instantiation is here
// in the .cc file, while the macro-spelling location is in the .h
// file at line 11.

#include "tests/forward_declare_in_macro.h"

// Important that this not be on line 11.  Note that this macro is
// carefully constructed so the beginning of it is in
// forward_declare_in_macro.h, while the end of it is in macro.cc
// (because the macro begins with 'normal' text and ends with a macro
// parameter).  If iwyu incorrectly uses spelling location, the macro
// will begin in forward_declare_in_macro.h:11 and end in
// forward_declare_in_macro.cc:<nextline>, which will show as iwyu
// thinking this forward-declaration spans many lines.  If iwyu
// correctly uses instantiation location, on the other hand, it will
// show as iwyu thinking this forward-declaration spans only one line.
FORWARD_DECLARE_CLASS(MyClass);

/**** IWYU_SUMMARY

tests/forward_declare_in_macro.cc should add these lines:

tests/forward_declare_in_macro.cc should remove these lines:
- class MyClass;  // lines XX-XX

The full include-list for tests/forward_declare_in_macro.cc:
#include "tests/forward_declare_in_macro.h"

***** IWYU_SUMMARY */
