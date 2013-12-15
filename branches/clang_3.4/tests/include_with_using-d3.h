//===--- include_with_using-d3.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace ns3 {
class PtrInNs3 {};
}
// The using decl for PtrInNs3 is in include_with_using-d3b.h
