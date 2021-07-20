//===--- ctad-i1.h - test input file for iwyu -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <class Func>
struct Deduced {
  Deduced(Func&& deferred) : deferred(deferred) {
  }

  ~Deduced() {
    deferred();
  }

  Func deferred;
};
