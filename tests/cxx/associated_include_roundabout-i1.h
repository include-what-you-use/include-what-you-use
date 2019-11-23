//===--- associated_include_roundabout-i1.h - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/associated_include_roundabout.h"

namespace hfile {
class NeedsItToo { RoundaboutClass aic; };
}

// No summary, because it's not part of the main compilation unit.

/**** IWYU_SUMMARY

***** IWYU_SUMMARY */
