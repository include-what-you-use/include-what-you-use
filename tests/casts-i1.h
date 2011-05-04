//===--- casts-i1.h - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we handle correctly handle the various types of cast.

class CastsClass { };

class CastsSubclass : public CastsClass { };
