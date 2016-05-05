//===--- direct_subfolder.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file includes and uses only another file in the subfolder

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_DIRECT_SUBFOLDER_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_DIRECT_SUBFOLDER_H_

#include "indirect_subfolder.h"

namespace { IndirectSubfolderClass foo; }

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_DIRECT_SUBFOLDER_H_
