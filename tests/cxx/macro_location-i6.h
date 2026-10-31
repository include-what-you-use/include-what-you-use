//===--- macro_location-i6.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This header does not provide FnDeclaredInPublicHeader, but it is provided by
// the public counterpart of this private header (i.e. '-i2.h').
#define CALL_FN_DECLARED_IN_PUBLIC_HEADER FnDeclaredInPublicHeader()
