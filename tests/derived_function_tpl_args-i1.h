//===--- derived_function_tpl_args-i1.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class IndirectClass { };

namespace ns {
class NsClass { };
}

template<typename T, typename U=ns::NsClass> class IndirectTplClass { };
