//===--- 987.h - iwyu test ------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cstdint>

void foo(std::uint8_t arg);

/**** IWYU_SUMMARY

(tests/bugs/987/987.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
