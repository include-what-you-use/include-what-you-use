//===--- reexport_overridden-i3.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class FwdRetType;
class IndirectClass;
template <typename>
class IndirectTemplate;

using Alias = IndirectClass;
using TemplatePtrAlias = IndirectTemplate<IndirectClass>*;

namespace ns {
using ::FwdRetType;
using ::IndirectClass;
}  // namespace ns

struct Struct {};
