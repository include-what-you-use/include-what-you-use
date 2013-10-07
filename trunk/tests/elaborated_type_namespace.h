//===--- elaborated_type_namespace.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace Elaboration {
  class Class {};

  template< typename T, typename U >
  struct Template {
    typedef T FirstType;
    typedef U SecondType;
  };
}
