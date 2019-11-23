//===--- associated_include_roundabout.cc - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that is we include headers that include our associated header, we still
// record it as belonging to the main compilation unit. In this case, our first
// header includes our associated header. Related issue:
// https://github.com/include-what-you-use/include-what-you-use/issues/738

#include "tests/cxx/associated_include_roundabout_1.h"
#include "tests/cxx/associated_include_roundabout.h"

namespace hfile {
AssociatedIncludeClass aic;
AnotherOne ao;
}

/**** IWYU_SUMMARY

tests/cxx/associated_include.cc should add these lines:

tests/cxx/associated_include.cc should remove these lines:

The full include-list for tests/cxx/associated_include.cc:
#include "tests/cxx/associated_include_roundabout.h"
#include "tests/cxx/associated_include_roundabout_1.h"

***** IWYU_SUMMARY */
