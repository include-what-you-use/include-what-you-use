//===--- 1220.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu -v3
// IWYU_XFAIL

#include <array> // IWYU pragma: keep
// IWYU pragma: no_include <cstddef>

std::size_t x = 123;

/**** IWYU_SUMMARY

(tests/bugs/1220/1220.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
