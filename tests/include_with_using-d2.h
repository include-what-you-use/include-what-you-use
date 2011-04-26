//===--- include_with_using-d1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This class isn't actually used by the .cc file.  The purpose of
// this code is to make sure that the .cc file doesn't see the need to
// #include -d2.h just because of the using declaration (the using
// declaration should actually be needed by the .cc file for that to
// happen).
namespace ns2 {
class PtrInNs2 {};
}

using ns2::PtrInNs2;

class UsedFromD2 { };
