//===--- macro_defined_by_includer_mapped-i1.h - test input file for iwyu -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Exported by -d1.h, which defines EXPORTED_VALUE_CHAR_T before including it.

inline bool operator==(ExportedValue a, ExportedValue b) {
  return a.v == b.v;
}

inline ExportedValue MakeExportedValue(EXPORTED_VALUE_CHAR_T c) {
  return {c};
}
