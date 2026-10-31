//===--- macro_location-i1.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_LOCATION_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_LOCATION_I1_H_

#define DECLARE_FN void MacroDeclaredFn();

// For the testing purpose, this header should be public, not private
// behind '-i2.h'.
void FnDeclaredInPublicHeader();

#define INTERNAL_MACRO InternalFnDeclaredAfterMacro()

void InternalFnDeclaredAfterMacro();

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_LOCATION_I1_H_
