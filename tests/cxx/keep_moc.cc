//===--- keep_moc.cc - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/*-i1.h" -I .

// Tests that IWYU never suggests to remove an include of a Qt .moc file.
// These files are handled by a separate Qt preprocessor (called 'moc'), and
// can't be analyzed in the normal C or C++ sense, The moc preprocessor does
// rudimentary IWYU analysis in its own universe.

// Out-of-the-blue include of a .moc, to make sure we never remove them.
#include "tests/cxx/keep_moc.moc"
#include "tests/cxx/keep_moc-d1.h"

void foo() {
  // IWYU: QObjectLike is...*keep_moc-i1.h
  QObjectLike x;
}

/**** IWYU_SUMMARY

tests/cxx/keep_moc.cc should add these lines:
#include "tests/cxx/keep_moc-i1.h"

tests/cxx/keep_moc.cc should remove these lines:
- #include "tests/cxx/keep_moc-d1.h"  // lines XX-XX

The full include-list for tests/cxx/keep_moc.cc:
#include "tests/cxx/keep_moc-i1.h"  // for QObjectLike
#include "tests/cxx/keep_moc.moc"

***** IWYU_SUMMARY */

