//===--- include_with_using-d5.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace ns5 {
class PtrInNs5 {};
}
// This using decl won't be used because it's not in the right namespace
namespace ns_unused {
using ns5::PtrInNs5;
}
