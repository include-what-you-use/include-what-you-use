//===--- exitcode_syntax_error.c - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that IWYU exits with code 1 when Clang fails to parse the provided
// source code.

// IWYU: expected ';' after top level declarator
// IWYU: unknown type name 'this'
this is not valid C code;

/**** IWYU_SUMMARY(1)

***** IWYU_SUMMARY */
