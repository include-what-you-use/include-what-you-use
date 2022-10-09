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

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_CATCH_EXCEPTIONS_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_CATCH_EXCEPTIONS_H_

#include "tests/cxx/catch-byptr.h"    // for CatchByPtr
#include "tests/cxx/catch-byref.h"    // for CatchByRef
#include "tests/cxx/catch-byvalue.h"  // for CatchByValye
#include "tests/cxx/catch-elab.h"     // for CatchElab
#include "tests/cxx/catch-logex.h"    // for LogException
#include "tests/cxx/catch-macro-1.h"  // for CatchMacro1
#include "tests/cxx/catch-macro-2.h"  // for CatchMacro2
#include "tests/cxx/catch-thrown.h"   // for Thrown

#include <stdio.h>

#endif // INCLUDE_WHAT_YOU_USE_TESTS_CXX_CATCH_EXCEPTIONS_H_
