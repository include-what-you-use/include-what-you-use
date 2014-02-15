//===--- fwd_declare_enum.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that iwyu does not barf when the user re-declares an enum.
// ("Forward" declare is not quite the right terminology, since you
// can only re-declare an enum *after* it's been defined, but it
// matches the terminology used in iwyu to refer to declarations that
// are not definitions.)

// Unused enum
enum Foo { FOO };
enum Foo;

// Used enum
enum Bar { BAR };
enum Bar;
Bar bar;


/**** IWYU_SUMMARY

(tests/cxx/fwd_declare_enum.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
