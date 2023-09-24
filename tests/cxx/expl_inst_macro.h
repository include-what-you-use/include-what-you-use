//===--- expl_inst_macro.h - test input file for iwyu ---*- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <iostream>

// This triggers use of an explicit instantiation basic_ostream<char> from
// <iostream> (with gcc/libstdc++), which in turn is processed by IWYU.
#define macro std::cout << " "

/**** IWYU_SUMMARY

(tests/cxx/expl_inst_macro.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
