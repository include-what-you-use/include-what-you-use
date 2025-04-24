//===--- 1720.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I tests/bugs/1720 \
//            -include public.h \
//            -Xiwyu --mapping_file=tests/bugs/1720/m.imp \
//            -Xiwyu --prefix_header_includes=remove
// IWYU_XFAIL

#include "another.h"

s32 val;
ComplexType other;

/**** IWYU_SUMMARY

(tests/bugs/1720/1720.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
