//===--- specialization_qualifier-inst.h - test input ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef SPECIALIZATION_QUALIFIER_INST_H_
#define SPECIALIZATION_QUALIFIER_INST_H_

#include "tests/cxx/indirect.h"
#include "tests/cxx/specialization_qualifier-indirect.h"

extern template struct Box<IndirectClass>;

#endif  // SPECIALIZATION_QUALIFIER_INST_H_
