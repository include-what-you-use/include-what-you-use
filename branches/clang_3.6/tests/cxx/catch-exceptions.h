//===--- catch-exceptions.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Facade header for exception types. We include this in catch.cc to provoke
// IWYU warnings and replacements.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_CATCH_EXCEPTIONS_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_CATCH_EXCEPTIONS_H_

#include "tests/cxx/catch-byptr.h"    // for CatchByPtr
#include "tests/cxx/catch-byref.h"    // for CatchByRef
#include "tests/cxx/catch-byvalue.h"  // for CatchByValye
#include "tests/cxx/catch-elab.h"     // for CatchElab
#include "tests/cxx/catch-logex.h"    // for LogException
#include "tests/cxx/catch-thrown.h"   // for Thrown

#include <stdio.h>

#endif // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_CATCH_EXCEPTIONS_H_
