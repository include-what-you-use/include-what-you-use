//===--- keep_embedded_include.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also=tests/cxx/*-d1.h \
//            -Xiwyu --check_also=tests/cxx/*-d2.h -I .

// Tests that IWYU keeps includes inside declarations.

enum Enum {
#include "tests/cxx/keep_embedded_include-d1.h"
};

enum EnumUsingMacro {
#include "tests/cxx/keep_embedded_include-d2.h"
};

int x = 1 +
#include "tests/cxx/keep_embedded_include-d3.h"
    ;

#include "tests/cxx/keep_embedded_include-d4.h"
i;

#include "tests/cxx/keep_embedded_include-d4.h"
j;

template <typename...>
class Tpl {};

Tpl<
#include "tests/cxx/keep_embedded_include-d5.h"
    >
    t;

/**** IWYU_SUMMARY

(tests/cxx/keep_embedded_include.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
