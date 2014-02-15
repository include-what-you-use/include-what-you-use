//===--- system_namespaces-d1.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// We're not supposed to define anything in std, but we do anyway for this test.

namespace std {
class StdClass { };
}
