//===--- using_in_pch.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that a using declaration in a precompiled header won't crash the app

void use_the_using_declaration()
{
  A a;
}

/**** IWYU_SUMMARY

***** IWYU_SUMMARY */