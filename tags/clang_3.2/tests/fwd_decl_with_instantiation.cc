//===--- fwd_decl_with_instantiation.cc - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests a situation where iwyu got confused between forward-declarations
// and definitions.  It happened when the .cc file needed a definition of
// a templated type and the .h only needed a forward-declaration.  The
// .h file would replace the forward-declaration with another one
// exactly like it.

#include "tests/fwd_decl_with_instantiation.h"
#include "tests/fwd_decl_with_instantiation-d1.h"

DirectClass<int> instantiated_dc;


/**** IWYU_SUMMARY

(tests/fwd_decl_with_instantiation.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
