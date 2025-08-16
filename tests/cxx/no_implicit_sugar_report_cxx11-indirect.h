//===--- no_implicit_sugar_report_cxx11-indirect.h - test input -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct TypedefHost {
  typedef int Int;
};

class Class;

namespace ns {
using ::Class;
}
