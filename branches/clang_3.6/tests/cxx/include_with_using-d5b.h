//===--- include_with_using-d5b.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This using decl *will* be used because it's in the right namespace
namespace ns_cc {
using ns5::PtrInNs5;
}
