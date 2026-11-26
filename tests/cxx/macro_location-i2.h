//===--- macro_location-i2.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/macro_location-i1.h"
#include "tests/cxx/macro_location-i6.h"

DECLARE_FN

void FnWithRedecl();

#define CALL_FN_DECLARED_AFTER_MACRO ProvidedFnDeclaredAfterMacro()

void ProvidedFnDeclaredAfterMacro();
