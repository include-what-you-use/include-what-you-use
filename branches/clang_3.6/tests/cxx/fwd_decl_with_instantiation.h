//===--- fwd_decl_with_instantiation.h - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template<typename T> class DirectClass;

DirectClass<int>* dc = 0;

/**** IWYU_SUMMARY

(tests/cxx/fwd_decl_with_instantiation.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
