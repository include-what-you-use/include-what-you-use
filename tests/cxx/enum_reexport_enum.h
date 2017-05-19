//===--- enum_reexport_enum.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Enum nested in class.
class Unscoped {
public:
  enum Enum {
    V1,
    V2,
    V3
  };
};

// Global enum.
enum UnscopedEnum {
  UE_V1,
  UE_V2,
  UE_V3
};
