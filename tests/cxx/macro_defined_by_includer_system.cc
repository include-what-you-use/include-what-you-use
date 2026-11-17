//===--- macro_defined_by_includer_system.cc - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -isystem tests/cxx/macro_defined_by_includer_system

// Tests a header that defines a macro before including a system header, to
// opt in to part of that header's API.  The includer provides only the
// declarations that depend on the macro: other files that use the rest of
// the system header must keep including it directly.

#include "tests/cxx/macro_defined_by_includer_system-d1.h"
#include <opt_in_api.h>

ConfiguredUser user;
OptInBase base;
// Declared only when the includer defines OPT_IN_API_EXTENDED.
OptInExtended extended;

// CONFIGURED_API_WIDE changes ConfiguredValue, so the includer that defines
// it provides everything in <configured_api.h>, even declarations outside
// the conditional block.
ConfiguredObject object;

/**** IWYU_SUMMARY

(tests/cxx/macro_defined_by_includer_system.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
