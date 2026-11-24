//===--- specialization_use_ptr_only.cc - test input ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 -Xiwyu --verbose=1 \
//            -Xiwyu --max_line_length=85 \
//            -Xiwyu --check_also=tests/cxx/specialization_use_ptr_only-macro.h

// Tests that repeated specialization reports count as often as they are made
// in '(ptr only)' comments, too. Each use of Plain in Holder<Plain> also
// reports a use of the forward-declaration in the file that defines the
// macro, which -plain.h satisfies, so -macro.h's include of -plain.h is for
// Plain (ptr only), more than fifty times when every revisit of a
// specialization type counts, and for Other, twelve times. Summary matching
// sorts comment names, but below verbose level 3 IWYU truncates comments to
// the maximum line length, which here leaves room for the most frequent
// symbol only.

#include "tests/cxx/specialization_use_ptr_only-macro.h"
#include "tests/cxx/specialization_use_ptr_only-plain.h"

MAKE_HOLDER(holder)

/**** IWYU_SUMMARY

tests/cxx/specialization_use_ptr_only.cc should add these lines:
#include "tests/cxx/specialization_use_ptr_only-root.h"

tests/cxx/specialization_use_ptr_only.cc should remove these lines:

The full include-list for tests/cxx/specialization_use_ptr_only.cc:
#include "tests/cxx/specialization_use_ptr_only-macro.h"  // for MAKE_HOLDER
#include "tests/cxx/specialization_use_ptr_only-plain.h"  // for Plain
#include "tests/cxx/specialization_use_ptr_only-root.h"  // for Holder

***** IWYU_SUMMARY */
