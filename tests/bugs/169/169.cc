//===--- 169.cc - test input file for iwyu --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -stdlib=libc++
// IWYU_XFAIL

#include <type_traits>
#include "type.h"

std::remove_pointer<Type*>::type x;

/**** IWYU_SUMMARY

(tests/bugs/169/169.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
