//===--- integer_template_arg.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a test to make sure a Clang crashing bug doesn't regress.

template <int Arg> class Base {};
class Derived : public Base<1> {};

/**** IWYU_SUMMARY

(tests/cxx/integer_template_arg.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
