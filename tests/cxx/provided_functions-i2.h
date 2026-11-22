//===--- provided_functions-i2.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct Indirect {
  friend bool operator<(const Indirect&, const Indirect&) {
    return false;
  }
};

void swap(Indirect&, Indirect&);

bool operator==(const Indirect&, const Indirect&);
bool operator==(const Indirect&, int);
bool operator==(int, const Indirect&);

void TakeByValue(Indirect);

class Unrelated {};
